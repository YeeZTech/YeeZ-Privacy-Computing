#include "ypc/common/crypto_prefix.h"
#include "ypc/common/limits.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/privacy_data_reader.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/version.h"
#include "ypc/corecommon/blockfile/blockfile_v1.h"
#include "ypc/corecommon/blockfile/blockfile_v2.h"
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
  virtual uint32_t decrypt_message_with_prefix(const ypc::bytes &private_key,
                                               const ypc::bytes &cipher,
                                               ypc::bytes &data,
                                               uint32_t prefix) = 0;
};
using crypto_ptr_t = std::shared_ptr<crypto_base>;
template <typename Crypto> class crypto_tool : public crypto_base {
public:
  using crypto_t = Crypto;
  virtual uint32_t decrypt_message_with_prefix(const ypc::bytes &private_key,
                                               const ypc::bytes &cipher,
                                               ypc::bytes &data,
                                               uint32_t prefix) {
    return crypto_t::decrypt_message_with_prefix(private_key, cipher, data,
                                                 prefix);
  }
};

uint32_t unseal_file(const crypto_ptr_t &crypto_ptr,
                     const ypc::bytes &private_key,
                     const std::string &sealed_file_path,
                     const std::string &file) {
  std::ifstream ifs;
  ifs.open(sealed_file_path.c_str(), std::ios::in | std::ios::binary);
  if (!ifs.is_open()) {
    throw std::invalid_argument(
        boost::str(boost::format("open file %1% failed!") % sealed_file_path));
  }
  // header: magic_number, version_number, block_number, item_number
  // item_number starts with 24 bytes offset
  ypc::internal::blockfile_header_v2 header{};
  // ifs.seekg(0, ifs.beg);
  ifs.seekg(-sizeof(header), ifs.end);
  ifs.read((char *)&header, sizeof(header));
  if (!ifs) {
    throw std::invalid_argument("read header failed!");
  }
  std::cout << "block number: " << header.block_number << std::endl;
  uint64_t item_number = header.item_number;
  std::cout << "item number: " << header.item_number << std::endl;
  // block info: 32bytes
  ypc::internal::blockfile_header_v1 bi{};
  // auto offset = sizeof(header);
  auto offset = -(sizeof(header) + header.block_number * 32);
  for (int i = 0; i < header.block_number; i++) {
    // ifs.seekg(offset + 32 * i, ifs.beg);
    ifs.seekg(offset + 32 * i, ifs.end);
    ifs.read((char *)&bi, sizeof(bi));
    // std::cout << "block: " << i + 1 << std::endl;
    // std::cout << "start_item_index: " << bi.magic_number << std::endl;
    // std::cout << "end_item_index: " << bi.version_number << std::endl;
    // std::cout << "start_file_pos: " << bi.block_number << std::endl;
    // std::cout << "end_file_pos: " << bi.item_number << std::endl;
  }
  ifs.close();
  ypc::simple_sealed_file sf(sealed_file_path, true);
  ypc::bytes buf(256 * ypc::utc::max_item_size);
  while (0u != item_number--) {
    size_t len;
    bool ret = sf.next_item((char *)buf.data(), buf.size(), len) ==
               ypc::simple_sealed_file::blockfile_t::succ;
    if (ret) {
      ypc::bytes cipher(len);
      memcpy(cipher.data(), buf.data(), len);
      ypc::bytes batch;
      auto status = crypto_ptr->decrypt_message_with_prefix(
          private_key, cipher, batch, ypc::utc::crypto_prefix_arbitrary);
      if (0u != status) {
        throw std::invalid_argument(boost::str(
            boost::format("decrypt batch %1% failed!") % item_number));
      }
      std::cout << "decrypt item succ!" << std::endl;
      std::cout << "batch: " << batch << std::endl;
      auto pkg = ypc::make_package<ntt::batch_data_pkg_t>::from_bytes(batch);
      auto batch_data = pkg.get<ntt::batch_data>();
      for (auto &l : batch_data) {
        std::cout << l << std::endl;
      }
    }
  }
  return 0;
}

const char *arg_crypto = "crypto";
const char *arg_use_privatekey_file = "use-privatekey-file";
const char *arg_use_privatekey_hex = "use-privatekey-hex";
const char *arg_sealed_data_url = "sealed-data-url";
const char *arg_plugin_path = "plugin-path";
const char *arg_data_url = "data-url";

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Data Hub options");
  bp::options_description general("General Options");
  bp::options_description seal_data_opts("Seal Data Options");

  // clang-format off
  seal_data_opts.add_options()
    (arg_crypto, bp::value<std::string>()->default_value("stdeth"), "choose the crypto, stdeth/gmssl")
    (arg_use_privatekey_file, bp::value<std::string>(), "private key file")
    (arg_use_privatekey_hex, bp::value<std::string>(), "private key")
    (arg_sealed_data_url, bp::value<std::string>(), "Sealed data URL")
    (arg_plugin_path, bp::value<std::string>(), "shared library for reading data")
    (arg_data_url, bp::value<std::string>(), "Data URL");

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
  if (vm.count(arg_crypto) == 0u) {
    std::cerr << "crypto not specified" << std::endl;
    return -1;
  }
  if ((vm.count(arg_use_privatekey_file) == 0u) &&
      (vm.count(arg_use_privatekey_hex) == 0u)) {
    std::cerr << "missing private key, use 'use-privatekey-file' or "
                 "'use-privatekey-hex'"
              << std::endl;
    return -1;
  }
  if (vm.count(arg_sealed_data_url) == 0u) {
    std::cerr << "sealed data url not specified" << std::endl;
    return -1;
  }
  if (vm.count(arg_data_url) == 0u) {
    std::cerr << "data not specified!" << std::endl;
    return -1;
  }

  ypc::bytes private_key;
  if (vm.count(arg_use_privatekey_hex) != 0u) {
    private_key = ypc::hex_bytes(vm[arg_use_privatekey_hex].as<std::string>())
                      .as<ypc::bytes>();
  } else if (vm.count(arg_use_privatekey_file) != 0u) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm[arg_use_privatekey_file].as<std::string>(), pt);
    private_key = pt.get<ypc::bytes>("private-key");
  }

  std::string crypto = vm[arg_crypto].as<std::string>();
  std::string sealed_data_file = vm[arg_sealed_data_url].as<std::string>();
  std::string data_file = vm[arg_data_url].as<std::string>();

  crypto_ptr_t crypto_ptr;
  if (crypto == "stdeth") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
  } else if (crypto == "gmssl") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
  } else {
    throw std::runtime_error("Unsupperted crypto type!");
  }

  auto status =
      unseal_file(crypto_ptr, private_key, sealed_data_file, data_file);
  if (status != 0u) {
    return -1;
  }

  std::cout << "done unsealing" << std::endl;
  return 0;
}
