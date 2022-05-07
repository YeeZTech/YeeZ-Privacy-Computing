#include "common/crypto_prefix.h"
#include "common/limits.h"
#include "corecommon/crypto/stdeth.h"
#include "corecommon/nt_cols.h"
#include "stbox/eth/eth_hash.h"
#include "ypc/ntobject_file.h"
#include "ypc/privacy_data_reader.h"
#include "ypc/sealed_file.h"
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

typedef ypc::crypto::eth_sgx_crypto crypto_t;
typedef ypc::nt<ypc::bytes> ntt;
void write_batch(simple_sealed_file &sf, const std::vector<ypc::bytes> &batch,
                 const stbox::bytes &public_key) {
  ntt::batch_data_pkg_t pkg;
  ypc::bytes s;
  ypc::bytes batch_str =
      ypc::make_bytes<ypc::bytes>::for_package<ntt::batch_data_pkg_t,
                                               ntt::batch_data>(batch);
  uint32_t status = crypto_t::encrypt_message_with_prefix(
      public_key, batch_str, ypc::utc::crypto_prefix_arbitrary, s);
  if (status) {
    std::stringstream ss;
    ss << "encrypt "
       << " data fail: " << stbox::status_string(status);
    LOG(ERROR) << ss.str();
    std::cerr << ss.str();
    exit(1);
  }
  sf.write_item(s);
}
uint32_t seal_file(const std::string &plugin, const std::string &file,
                   const std::string &sealed_file_path,
                   const stbox::bytes &public_key, stbox::bytes &data_hash) {
  // Read origin file use sgx to seal file
  privacy_data_reader reader(plugin, file);
  simple_sealed_file sf(sealed_file_path, false);
  // std::string k(file);
  // k = k + std::string(sealer_path);

  // magic string here!
  data_hash = stbox::eth::keccak256_hash(bytes("Fidelius"));

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
      write_batch(sf, batch, public_key);
      batch.clear();
      batch_size = 0;
    }

    stbox::bytes k = data_hash + item_data;
    data_hash = stbox::eth::keccak256_hash(k);
    item_data = reader.read_item_data();
    if (item_data.size() > ypc::utc::max_item_size) {
      std::cerr << "only support item size that smaller than "
                << ypc::utc::max_item_size << " bytes!" << std::endl;
      return 1;
    }
    ++pd;
    ++counter;
  }
  if (batch.size() != 0) {
    write_batch(sf, batch, public_key);
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
    ("data-url", bp::value<std::string>(), "Data URL")
    ("plugin-path", bp::value<std::string>(), "shared library for reading data")
    ("use-publickey-file", bp::value<std::string>(), "public key file")
    ("use-publickey-hex", bp::value<std::string>(), "public key")
    ("sealed-data-url", bp::value<std::string>(), "Sealed data URL")
    ("output", bp::value<std::string>(), "output meta file path");

  general.add_options()
    ("help", "help message");

  // clang-format on

  all.add(general).add(seal_data_opts);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
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
  if (!vm.count("data-url")) {
    std::cerr << "data not specified!" << std::endl;
    return -1;
    }
  if (!vm.count("sealed-data-url")) {
    std::cerr << "sealed data url not specified" << std::endl;
    return -1;
  }
  if (!vm.count("output")) {
    std::cerr << "output not specified" << std::endl;
    return -1;
  }
  if (!vm.count("plugin-path")) {
    std::cerr << "library not specified" << std::endl;
    return -1;
  }
  if (!vm.count("use-publickey-hex") && !vm.count("use-publickey-file")) {
    std::cerr << "missing public key, use 'use-publickey-file' or "
                 "'use-publickey-hex'"
              << std::endl;
    return -1;
  }

  ypc::bytes public_key;
  if (vm.count("use-publickey-hex")) {
    public_key = ypc::hex_bytes(vm["use-publickey-hex"].as<std::string>())
                     .as<ypc::bytes>();
  } else if (vm.count("use-publickey-file")) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm["use-publickey-file"].as<std::string>(), pt);
    public_key = pt.get<ypc::bytes>("public-key");
  }

  std::string plugin = vm["plugin-path"].as<std::string>();
  std::string data_file = vm["data-url"].as<std::string>();
  std::string output = vm["output"].as<std::string>();
  std::string sealed_data_file = vm["sealed-data-url"].as<std::string>();

  stbox::bytes data_hash;
  std::ofstream ofs;
  ofs.open(output);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs.close();

  auto status =
      seal_file(plugin, data_file, sealed_data_file, public_key, data_hash);
  if (status) {
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
  if (sample.size() > 0) {
    ofs << "sample_data"
        << " = " << sample << "\n";
  }
  std::string format = reader.get_data_format();
  if (format.size() > 0) {
    ofs << " data_fromat"
        << " = " << format << "\n";
  }
  ofs.close();

  std::cout << "done sealing" << std::endl;
  return 0;
}
