#include "ypc/common/crypto_prefix.h"
#include "ypc/common/limits.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/privacy_data_reader.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/version.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/nt_cols.h"
#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <thread>

using stx_status = stbox::stx_status;
using namespace ypc;
using ntt = ypc::nt<ypc::bytes>;

class crypto_base {
public:
  virtual uint32_t encrypt_message_with_prefix(const ypc::bytes &public_key,
                                               const ypc::bytes &data,
                                               uint32_t prefix,
                                               ypc::bytes &cipher) = 0;
  virtual uint32_t hash_256(const ypc::bytes &msg, ypc::bytes &hash) = 0;
};
using crypto_ptr_t = std::shared_ptr<crypto_base>;
template <typename Crypto> class crypto_tool : public crypto_base {
public:
  using crypto_t = Crypto;
  virtual uint32_t encrypt_message_with_prefix(const ypc::bytes &public_key,
                                               const ypc::bytes &data,
                                               uint32_t prefix,
                                               ypc::bytes &cipher) {
    return crypto_t::encrypt_message_with_prefix(public_key, data, prefix,
                                                 cipher);
  }
  virtual uint32_t hash_256(const ypc::bytes &msg, ypc::bytes &hash) {
    return crypto_t::hash_256(msg, hash);
  }
};

void write_batch(const crypto_ptr_t &crypto_ptr, simple_sealed_file &sf,
                 const std::vector<ypc::bytes> &batch,
                 const ypc::bytes &public_key) {
  ntt::batch_data_pkg_t pkg;
  ypc::bytes s;
  ypc::bytes batch_str =
      ypc::make_bytes<ypc::bytes>::for_package<ntt::batch_data_pkg_t,
                                               ntt::batch_data>(batch);
  uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, batch_str, ypc::utc::crypto_prefix_arbitrary, s);
  if (status != 0u) {
    std::stringstream ss;
    ss << "encrypt "
       << " data fail: " << stbox::status_string(status);
    LOG(ERROR) << ss.str();
    std::cerr << ss.str();
    exit(1);
  }
  sf.write_item(s);
}
uint32_t seal_file(const crypto_ptr_t &crypto_ptr, const std::string &plugin,
                   const std::string &file, const std::string &sealed_file_path,
                   const ypc::bytes &public_key, ypc::bytes &data_hash) {
  // Read origin file use sgx to seal file
  privacy_data_reader reader(plugin, file);
  simple_sealed_file sf(sealed_file_path, false);
  // std::string k(file);
  // k = k + std::string(sealer_path);

  // magic string here!
  crypto_ptr->hash_256(bytes("Fidelius"), data_hash);

  bytes item_data = reader.read_item_data();
  if (item_data.size() > ypc::utc::max_item_size) {
    std::cerr << "only support item size that smaller than "
              << ypc::utc::max_item_size << " bytes!" << std::endl;
    return 1;
  }
  uint64_t item_number = reader.get_item_number();

  std::cout << "Reading " << item_number << " items ..." << std::endl;
  boost::progress_display pd(item_number);
  uint counter = 0;
  std::vector<ypc::bytes> batch;
  size_t batch_size = 0;
  while (!item_data.empty() && counter < item_number) {
    batch.push_back(item_data);
    batch_size += item_data.size();
    if (batch_size >= ypc::utc::max_item_size) {
      write_batch(crypto_ptr, sf, batch, public_key);
      batch.clear();
      batch_size = 0;
    }

    ypc::bytes k = data_hash + item_data;
    crypto_ptr->hash_256(k, data_hash);

    item_data = reader.read_item_data();
    if (item_data.size() > ypc::utc::max_item_size) {
      std::cerr << "only support item size that smaller than "
                << ypc::utc::max_item_size << " bytes!" << std::endl;
      return 1;
    }
    ++pd;
    ++counter;
  }
  if (!batch.empty()) {
    write_batch(crypto_ptr, sf, batch, public_key);
    batch.clear();
    batch_size = 0;
  }

  std::cout << "data hash: " << data_hash << std::endl;
  std::cout << "\nDone read data count: " << pd.count() << std::endl;
  return 0;
}

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Data Hub options");
  bp::options_description general("General Options");
  bp::options_description seal_data_opts("Seal Data Options");

  // clang-format off
  seal_data_opts.add_options()
    ("crypto", bp::value<std::string>()->default_value("stdeth"), "choose the crypto, stdeth/gmssl")
    ("data-url", bp::value<std::string>(), "Data URL")
    ("plugin-path", bp::value<std::string>(), "shared library for reading data")
    ("use-publickey-file", bp::value<std::string>(), "public key file")
    ("use-publickey-hex", bp::value<std::string>(), "public key")
    ("sealed-data-url", bp::value<std::string>(), "Sealed data URL")
    ("output", bp::value<std::string>(), "output meta file path");


  general.add_options()
    ("help", "help message")
    ("version", "show version");

  // clang-format on

  all.add(general).add(seal_data_opts);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help") != 0u) {
    std::cout << all << std::endl;
    exit(-1);
  }
  if (vm.count("version") != 0u) {
    std::cout << ypc::get_ypc_version() << std::endl;
    exit(-1);
  }
  return vm;
}

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }
  if (vm.count("data-url") == 0u) {
    std::cerr << "data not specified!" << std::endl;
    return -1;
    }
  if (vm.count("sealed-data-url") == 0u) {
    std::cerr << "sealed data url not specified" << std::endl;
    return -1;
  }
  if (vm.count("output") == 0u) {
    std::cerr << "output not specified" << std::endl;
    return -1;
  }
  if (vm.count("plugin-path") == 0u) {
    std::cerr << "library not specified" << std::endl;
    return -1;
  }
  if ((vm.count("use-publickey-hex") == 0u) && (vm.count("use-publickey-file") == 0u)) {
    std::cerr << "missing public key, use 'use-publickey-file' or "
                 "'use-publickey-hex'"
              << std::endl;
    return -1;
  }
  if (vm.count("crypto") == 0u) {
    std::cerr << "crypto not specified" << std::endl;
    return -1;
  }

  ypc::bytes public_key;
  if (vm.count("use-publickey-hex") != 0u) {
    public_key = ypc::hex_bytes(vm["use-publickey-hex"].as<std::string>())
                     .as<ypc::bytes>();
  } else if (vm.count("use-publickey-file") != 0u) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm["use-publickey-file"].as<std::string>(), pt);
    public_key = pt.get<ypc::bytes>("public-key");
  }

  std::string plugin = vm["plugin-path"].as<std::string>();
  std::string data_file = vm["data-url"].as<std::string>();
  std::string output = vm["output"].as<std::string>();
  std::string sealed_data_file = vm["sealed-data-url"].as<std::string>();
  std::string crypto = vm["crypto"].as<std::string>();

  ypc::bytes data_hash;
  std::ofstream ofs;
  ofs.open(output);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs.close();

  crypto_ptr_t crypto_ptr;
  if (crypto == "stdeth") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
  } else if (crypto == "gmssl") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
  } else {
    throw std::runtime_error("Unsupperted crypto type!");
  }

  auto status = seal_file(crypto_ptr, plugin, data_file, sealed_data_file,
                          public_key, data_hash);
  if (status != 0u) {
    return -1;
  }

  ofs.open(output);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs << "data_url"
      << " = " << data_file << "\n";
  ofs << "sealed_data_url"
      << " = " << sealed_data_file << "\n";
  ofs << "public_key"
      << " = " << public_key << "\n";
  ofs << "data_id"
      << " = " << data_hash << "\n";

  privacy_data_reader reader(plugin, data_file);
  ofs << "item_num"
      << " = " << reader.get_item_number() << "\n";

  // sample and format are optional
  bytes sample = reader.get_sample_data();
  if (!sample.empty()) {
    ofs << "sample_data"
        << " = " << sample << "\n";
  }
  std::string format = reader.get_data_format();
  if (!format.empty()) {
    ofs << " data_fromat"
        << " = " << format << "\n";
  }
  ofs.close();

  std::cout << "done sealing" << std::endl;
  return 0;
}
