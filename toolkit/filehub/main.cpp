#include "ypc/common/crypto_prefix.h"
#include "ypc/common/limits.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/privacy_data_reader.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/version.h"
#include "ypc/corecommon/blockfile/blockfile_v1.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/nt_cols.h"
#include "thread_pool_r.h"
#include "serialize_utils.h"

#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <thread>

using stx_status = stbox::stx_status;
using namespace ypc;
using ntt = ypc::nt<ypc::bytes>;

class crypto_base
{
public:
  virtual uint32_t encrypt_message_with_prefix(const ypc::bytes &public_key,
                                               const ypc::bytes &data,
                                               uint32_t prefix,
                                               ypc::bytes &cipher) = 0;
  virtual uint32_t hash_256(const ypc::bytes &msg, ypc::bytes &hash) = 0;
};
using crypto_ptr_t = std::shared_ptr<crypto_base>;
template <typename Crypto>
class crypto_tool : public crypto_base
{
public:
  using crypto_t = Crypto;
  virtual uint32_t encrypt_message_with_prefix(const ypc::bytes &public_key,
                                               const ypc::bytes &data,
                                               uint32_t prefix,
                                               ypc::bytes &cipher)
  {
    return crypto_t::encrypt_message_with_prefix(public_key, data, prefix,
                                                 cipher);
  }
  virtual uint32_t hash_256(const ypc::bytes &msg, ypc::bytes &hash)
  {
    return crypto_t::hash_256(msg, hash);
  }
};

void write_batch(const crypto_ptr_t &crypto_ptr, simple_sealed_file &sf,
                 const std::vector<ypc::bytes> &batch,
                 const ypc::bytes &public_key)
{
  ntt::batch_data_pkg_t pkg;
  ypc::bytes s;
  ypc::bytes batch_str =
      ypc::make_bytes<ypc::bytes>::for_package<ntt::batch_data_pkg_t,
                                               ntt::batch_data>(batch);
  // std::cout << "batch data: " << batch_str << std::endl;
  uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, batch_str, ypc::utc::crypto_prefix_arbitrary, s);
  if (status != 0u)
  {
    std::stringstream ss;
    ss << "encrypt "
       << " data fail: " << stbox::status_string(status);
    LOG(ERROR) << ss.str();
    std::cerr << ss.str();
    exit(1);
  }
  sf.write_item(s);
}

ypc::bytes seal_file(const crypto_ptr_t &crypto_ptr, const std::string &plugin,
                   const std::string &file, const std::string &sealed_file_path,
                   const ypc::bytes &public_key)
{
  // Read origin file use sgx to seal file
  privacy_data_reader reader(plugin, file);
  simple_sealed_file sf(sealed_file_path, false);
  // std::string k(file);
  // k = k + std::string(sealer_path);

  // magic string here!
  ypc::bytes data_hash;
  crypto_ptr->hash_256(bytes("Fidelius"), data_hash);

  bytes item_data = reader.read_item_data();
  if (item_data.size() > ypc::utc::max_item_size)
  {
    std::cerr << "only support item size that smaller than "
              << ypc::utc::max_item_size << " bytes!" << std::endl;
    return ypc::bytes();
  }
  uint64_t item_number = reader.get_item_number();

  // std::cout << "Reading " << item_number << " items ..." << std::endl;
  boost::progress_display pd(item_number);
  uint counter = 0;
  std::vector<ypc::bytes> batch;
  size_t batch_size = 0;
  while (!item_data.empty() && counter < item_number)
  {
    batch.push_back(item_data);
    batch_size += item_data.size();
    if (batch_size >= ypc::utc::max_item_size)
    {
      write_batch(crypto_ptr, sf, batch, public_key);
      batch.clear();
      batch_size = 0;
    }

    // std::cout << "item data: " << item_data << std::endl;
    ypc::bytes k = data_hash + item_data;
    crypto_ptr->hash_256(k, data_hash);

    item_data = reader.read_item_data();
    if (item_data.size() > ypc::utc::max_item_size)
    {
      std::cerr << "only support item size that smaller than "
                << ypc::utc::max_item_size << " bytes!" << std::endl;
      return ypc::bytes();
    }
    ++pd;
    ++counter;
  }
  if (!batch.empty())
  {
    write_batch(crypto_ptr, sf, batch, public_key);
    batch.clear();
    batch_size = 0;
  }

  // std::cout << "data hash: " << data_hash << std::endl;
  // std::cout << "Done read data count: " << pd.count() << std::endl;
  return data_hash;
}

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[])
{
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Data Hub options");
  bp::options_description general("General Options");
  bp::options_description seal_data_opts("Seal Data Options");

  // clang-format off
  seal_data_opts.add_options()
    ("crypto", bp::value<std::string>()->default_value("stdeth"), "choose the crypto, stdeth/gmssl")
    ("use-publickey-file", bp::value<std::string>(), "public key file")
    ("use-publickey-hex", bp::value<std::string>(), "public key")
    ("data-url", bp::value<std::string>(), "Data URL")
    ("plugin-path", bp::value<std::string>(), "shared library for reading data")
    ("sealed-data-url", bp::value<std::string>(), "Sealed data URL")
    ("output", bp::value<std::string>(), "output meta file path")
    ("thread-num", bp::value<int>()->default_value(1), "thread number");


  general.add_options()
    ("help", "help message")
    ("version", "show version");
  // clang-format on

  all.add(general).add(seal_data_opts);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help") != 0u)
  {
    std::cout << all << std::endl;
    exit(-1);
  }
  if (vm.count("version") != 0u)
  {
    std::cout << ypc::get_ypc_version() << std::endl;
    exit(-1);
  }
  return vm;
}

int main(int argc, char *argv[])
{
  boost::program_options::variables_map vm;
  try
  {
    vm = parse_command_line(argc, argv);
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }
  if (vm.count("crypto") == 0u)
  {
    std::cerr << "crypto not specified" << std::endl;
    return -1;
  }
  if ((vm.count("use-publickey-hex") == 0u) && (vm.count("use-publickey-file") == 0u))
  {
    std::cerr << "missing public key, use 'use-publickey-file' or "
                 "'use-publickey-hex'"
              << std::endl;
    return -1;
  }
  if (vm.count("data-url") == 0u)
  {
    std::cerr << "data not specified!" << std::endl;
    return -1;
  }
  if (vm.count("plugin-path") == 0u)
  {
    std::cerr << "library not specified" << std::endl;
    return -1;
  }
  if (vm.count("sealed-data-url") == 0u)
  {
    std::cerr << "sealed data url not specified" << std::endl;
    return -1;
  }
  if (vm.count("output") == 0u)
  {
    std::cerr << "output not specified" << std::endl;
    return -1;
  }
  if(vm.count("thread-num") == 0u) {
    std::cerr << "thread-num not specified" << std::endl;
    return -1;
  }

  ypc::bytes public_key;
  if (vm.count("use-publickey-hex") != 0u)
  {
    public_key = ypc::hex_bytes(vm["use-publickey-hex"].as<std::string>())
                     .as<ypc::bytes>();
  }
  else if (vm.count("use-publickey-file") != 0u)
  {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm["use-publickey-file"].as<std::string>(), pt);
    public_key = pt.get<ypc::bytes>("public-key");
  }

  std::string crypto = vm["crypto"].as<std::string>();
  std::string rootDir = vm["data-url"].as<std::string>();
  std::string plugin = vm["plugin-path"].as<std::string>();
  std::string sealed_data_file = vm["sealed-data-url"].as<std::string>();
  std::string output = vm["output"].as<std::string>();
  int thread_num = vm["thread-num"].as<int>();
  if(thread_num <= 0) {
    std::cerr << "thread-num should > 0" << std::endl;
    return -1;
  }

  std::ofstream ofs;
  ofs.open(output);
  if (!ofs.is_open())
  {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs.close();

  crypto_ptr_t crypto_ptr;
  if (crypto == "stdeth")
  {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
  }
  else if (crypto == "gmssl")
  {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
  }
  else
  {
    throw std::runtime_error("Unsupperted crypto type!");
  }
  const boost::filesystem::path rootPath{rootDir};
  if (!boost::filesystem::exists(rootPath))
  {
    std::cerr << "Invalid directory path: " << rootDir << std::endl;
    return -1;
  }
  std::vector<boost::filesystem::path> filePath;
  if(boost::filesystem::is_directory(rootPath)) {
    for (boost::filesystem::recursive_directory_iterator it(rootPath), end; it != end; ++it)
    {
      if (boost::filesystem::is_regular_file(*it))
      {
        filePath.push_back(*it);
      }
    }
  } else if (boost::filesystem::is_regular_file(rootPath)) {
    filePath.push_back(rootPath);
  }
  // 返回值
  std::vector<std::future<ypc::bytes>> futures;

  // 多线程运行
  ThreadPool pool(thread_num);
  for (auto &file : filePath)
  {
    std::string data_file = file.string();
    std::string sealed_data = file.filename().string() + ".raw.sealed";
    // std::cout << "data file: " << data_file << " sealed file: " << sealed_data << std::endl;
    futures.push_back(pool.enqueue(seal_file, crypto_ptr, plugin, data_file, sealed_data, public_key));
  }

  std::ofstream seal_file_ofs;
  seal_file_ofs.open(sealed_data_file, std::ios::out | std::ios::binary);
  if (!seal_file_ofs.is_open())
  {
    std::cout << "Cannot open file " << sealed_data_file << "\n";
    return -1;
  }

  ypc::bytes all_data_hash;
  boost::property_tree::ptree root;
  for(int i = 0; i < futures.size(); i++){
    ypc::bytes data_hash = futures[i].get();
    std::string file_name = filePath[i].filename().string();
    if (data_hash.empty())
    {
      std::cout << "Failed to seal file " << file_name << "\n";
      seal_file_ofs.close();
      return -1;
    }
    all_data_hash += data_hash;
    std::string sealed_data = file_name + ".raw.sealed";
    std::ifstream seal_file_ifs(sealed_data, std::ios::in | std::ios::binary);
    if (!seal_file_ifs.is_open())
    {
      std::cout << "Cannot open file " << sealed_data << "\n";
      return -1;
    }
    seal_file_ifs.seekg(0, std::ios::end);
    std::streampos length = seal_file_ifs.tellg();
    seal_file_ifs.seekg(0, std::ios::beg);
    std::streampos offset = seal_file_ofs.tellp();
    // 使用缓冲区读取并写入数据
    std::vector<char> buffer(length);
    seal_file_ifs.read(buffer.data(), length);
    std::streamsize bytes_read = seal_file_ifs.gcount(); // 实际读取的字节数
    seal_file_ofs.write(buffer.data(), bytes_read);
    seal_file_ifs.close();
    std::string file_path = rootPath.filename().string() + "/" + (boost::filesystem::is_regular_file(rootPath) ?
                            "" : boost::filesystem::relative(filePath[i], rootPath).string());
    // std::cout << "file path: " << file_path << std::endl;
    datahub::insertFileInfo(root, file_path, length, offset);
    boost::filesystem::remove(sealed_data);
  }
  seal_file_ofs.close();
  
  // 写入目录结构
  datahub::serializeToBinaryFile(root, sealed_data_file);

  ofs.open(output);
  if (!ofs.is_open())
  {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs << "data_url"
      << " = " << rootDir << "\n";
  ofs << "sealed_data_url"
      << " = " << sealed_data_file << "\n";
  ofs << "public_key"
      << " = " << public_key << "\n";
  ofs << "data_id"
      << " = " << all_data_hash << "\n";

  ofs.close();

  std::cout << "done sealing" << std::endl;
  return 0;
}
