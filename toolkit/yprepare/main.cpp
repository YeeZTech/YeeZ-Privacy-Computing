#include "onchain_data_reader.h"
#include "ypc/filesystem.h"
#include "ypc/param_id.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>
#include <keymgr/common/util.h>
#include <keymgr/default/keymgr_sgx_module.h>
#include <stbox/ebyte.h>
#include <ypc/byte.h>
#include <ypc/sgx/parser_sgx_module.h>

#define ENCLAVE_KEYMGR_PATH "../lib/keymgr.signed.so"

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all(
      "YeeZ Privacy Data Explorer and Prepare for Analyzer");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("sample-path", bp::value<std::string>(), "sample json file path")
    ("dhash", bp::value<std::string>(), "data hash, show data info with hash")
    ("use-pubkey", bp::value<std::string>(), "local public key")
    ("use-param", bp::value<std::string>(), "param that need to be encrypted")
    ("param-format", bp::value<std::string>(), "[text|hex], default is [hex], param format")
    ("use-enclave", bp::value<std::string>(), "parser enclave path")
    ("output", bp::value<std::string>(), "output result to file with JSON format, only valid for '--use-pubkey' and '--use-param'")
    ("info", "show infomation")
    ("list", "show all data hashes");

  // clang-format on
  bp::variables_map vm;
  boost::program_options::store(bp::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }

  return vm;
}

uint32_t load_key_pair(const ypc::bytes &pkey, ypc::bytes &b_pkey,
                       ypc::bytes &b_skey) {
  uint32_t ret = 1;
  std::string key_dir = create_dir_if_not_exist(".", ".yeez.key/");
  boost::filesystem::path key_path(key_dir);
  if (!ypc::is_dir_exists(key_dir)) {
    throw std::runtime_error(
        boost::str(boost::format("Directory not exist %1%!") % key_dir));
  }

  std::string fname = std::string(
      (const char *)pkey.as<ypc::hex_bytes>().data(), PKEY_FILE_NAME_LENGTH);
  for (auto &f : boost::make_iterator_range(
           boost::filesystem::directory_iterator(key_path), {})) {
    auto name = f.path().filename().generic_string();
    if (fname == name) {
      boost::filesystem::path p = key_dir / boost::filesystem::path(name);
      ret = read_key_pair_from_file(p.generic_string(), b_pkey, b_skey);
      break;
    }
  }
  if (ret) {
    throw std::runtime_error(
        boost::str(boost::format("Key pair not found in path %1%") % key_dir));
  }
  return ret;
}

ypc::bytes encrypt_skey_with_pubkey(std::shared_ptr<keymgr_sgx_module> ptr,
                                    const ypc::bytes &pkey_for_encrypt,
                                    const ypc::bytes &pkey_for_skey,
                                    ypc::bytes &b_pkey,
                                    ypc::bytes &b_sealed_key) {
  load_key_pair(pkey_for_skey, b_pkey, b_sealed_key);
  ypc::bref backup_key;
  ptr->forward_private_key(b_sealed_key.data(), b_sealed_key.size(),
                           (const uint8_t *)pkey_for_encrypt.data(),
                           pkey_for_encrypt.size(), backup_key);

  return ypc::bytes(backup_key.data(), backup_key.size());
}

ypc::bytes encrypt_param_with_pubkey(std::shared_ptr<keymgr_sgx_module> ptr,
                                     const ypc::bytes &pubkey,
                                     const ypc::bytes &param) {
  ypc::bref cipher;
  ptr->encrypt_message(pubkey.data(), pubkey.size(), param.data(), param.size(),
                       cipher);

  return ypc::bytes(cipher.data(), cipher.size());
}

ypc::bytes generate_signature(std::shared_ptr<keymgr_sgx_module> ptr,
                              uint32_t msg_id, const ypc::bytes &cipher,
                              const ypc::bytes &epkey, const ypc::bytes &ehash,
                              const ypc::bytes &b_sealed_key,
                              const ypc::bytes &vpkey) {
  uint32_t all_size =
      sizeof(msg_id) + cipher.size() + epkey.size() + ehash.size();
  ypc::bytes all(all_size);
  memcpy(all.data(), &msg_id, sizeof(msg_id));
  memcpy(all.data() + sizeof(msg_id), cipher.data(), cipher.size());
  memcpy(all.data() + sizeof(msg_id) + cipher.size(), epkey.data(),
         epkey.size());
  memcpy(all.data() + sizeof(msg_id) + cipher.size() + epkey.size(),
         ehash.data(), ehash.size());
  ypc::bref sig;
  auto ret = ptr->sign_message(b_sealed_key.data(), b_sealed_key.size(),
                               all.data(), all_size, sig);
  if (ret) {
    throw std::runtime_error("Generate signature failed!");
  }
  std::cout << "Sign msg_id/cipher/epkey/ehash ---- Success" << std::endl;

  ret = ptr->verify_signature(all.data(), all.size(), sig.data(), sig.len(),
                              vpkey.data(), vpkey.size());
  if (ret) {
    throw std::runtime_error("Verify signature failed!");
  }
  std::cout << "Verify signature ---- Success" << std::endl;
  return ypc::bytes(sig.data(), sig.size());
}

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    std::cout << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  // TODO: we should refine the logic here, currently we use sample.json
  auto sample_json = boost::filesystem::path("../toolkit/yprepare/sample.json");
  if (vm.count("sample-path")) {
    sample_json = boost::filesystem::path(vm["sample-path"].as<std::string>());
  }
  std::unique_ptr<onchain_data_reader> odr =
      std::make_unique<dummy_onchain_data_reader>(sample_json.generic_string());
  odr->init();

  std::cout << "init onchain data done." << std::endl;
  if (vm.count("list")) {
    size_t index = 0;
    for (auto &c : odr->all_onchain_data()) {
      std::cout << "Index " << index << ": " << std::endl;
      std::cout << "\tData Hash: 0x" << c.get<data_hash>() << std::endl;
      std::cout << "\tProvider Public key: 0x" << c.get<provider_pub_key>()
                << std::endl;
      index++;
    }
    return 0;
  }

  if (!vm.count("dhash")) {
    std::cout << "No data hash is provided!" << std::endl;
    return -1;
  }
  ypc::bytes dhash =
      ypc::hex_bytes(vm["dhash"].as<std::string>()).as<ypc::bytes>();
  onchain_data_meta_t d;
  try {
    d = odr->get_data_with_hash(dhash);
    std::cout << "found data with hash: " << dhash << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Cannot find data with hash " << dhash << std::endl;
    return -1;
  }

  std::shared_ptr<keymgr_sgx_module> ptr;
  std::unordered_map<std::string, ypc::bytes> result;

  result["data-hash"] = d.get<data_hash>();
  result["provider-pkey"] = d.get<provider_pub_key>();

  if (vm.count("use-pubkey") || vm.count("use-param")) {
    ptr = std::make_shared<keymgr_sgx_module>(ENCLAVE_KEYMGR_PATH);
    std::cout << "init keymgr enclave done." << std::endl;
  }

  ypc::bytes b_pkey, b_sealed_key;
  if (vm.count("use-pubkey")) {
    auto pkey_for_encrypt = d.get<provider_pub_key>();
    auto pkey_for_skey =
        ypc::hex_bytes(vm["use-pubkey"].as<std::string>()).as<ypc::bytes>();
    auto es = encrypt_skey_with_pubkey(ptr, pkey_for_encrypt, pkey_for_skey,
                                       b_pkey, b_sealed_key);
    std::cout << "Encrypt private key ---- Success" << std::endl;
    result["encrypted-skey"] = es;
    result["analyzer-pkey"] =
        ypc::hex_bytes(vm["use-pubkey"].as<std::string>()).as<ypc::bytes>();
  }

  if (vm.count("use-param")) {
    if (!vm.count("use-pubkey")) {
      std::cout << "No local public key found! Please use '--use-pubkey' to "
                   "specify it."
                << std::endl;
      return -1;
    }

    std::string format = "hex";
    ypc::bytes param;
    if (vm.count("param-format")) {
      format = vm["param-format"].as<std::string>();
    }
    if (format == "hex") {
      param =
          ypc::hex_bytes(vm["use-param"].as<std::string>()).as<ypc::bytes>();
    } else if (format == "text") {
      param = ypc::bytes(vm["use-param"].as<std::string>().c_str());
    } else {
      std::cout << "unknow format from '--param-format='" << format
                << std::endl;
      return -1;
    }
    auto es = encrypt_param_with_pubkey(
        ptr,
        ypc::hex_bytes(vm["use-pubkey"].as<std::string>()).as<ypc::bytes>(),
        param);
    result["encrypted-input"] = es;
  }

  if (vm.count("use-enclave")) {
    std::string enclave_path =
        ypc::complete_path(vm["use-enclave"].as<std::string>());
    if (!vm.count("use-pubkey")) {
      std::cout << "No local public key found! Please use '--use-pubkey' to "
                   "specify it."
                << std::endl;
      return -1;
    }
    ypc::bref enclave_hash;
    parser_sgx_module mod(enclave_path.c_str());
    mod.get_enclave_hash(enclave_hash);

    // std::cout << "enclave hash: " << ypc::to_hex(enclave_hash) << std::endl;
    result["program-enclave-hash"] =
        ypc::bytes(enclave_hash.data(), enclave_hash.size());
  }

  // generate signature here
  ypc::bytes skey_sig =
      generate_signature(ptr, param_id::PRIVATE_KEY, result["encrypted-skey"],
                         result["provider-pkey"],
                         result["program-enclave-hash"], b_sealed_key, b_pkey);
  result["forward-sig"] = skey_sig;
  // std::string param_sig = generate_signature(ptr);

  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    for (auto it = result.begin(); it != result.end(); it++) {
      pt.put(it->first, it->second);
    }
    boost::property_tree::json_parser::write_json(output_path, pt);
  }
}
