#include "keymgr/common/util.h"
#include "keymgr_sgx_module.h"
#include "stbox/stx_common.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <sstream>

using namespace stbox;
boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Key Manager options");
  bp::options_description backup("YeeZ Backup options");
  bp::options_description restore("YeeZ Restore options");
  bp::options_description encrypt("YeeZ Encrypt options");
  bp::options_description decrypt("YeeZ Decrypt options");
  bp::options_description sign("YeeZ Sign options");
  bp::options_description verify("YeeZ Verify options");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("create", "create a secp256k1 key pair")
    ("list", "list secp256k1 keys")
    ("remove", bp::value<std::string>(), "remove a secp256k1 key pair")
    ("backup", bp::value<std::string>(), "backup a secp256k1 private key")
    ("restore", bp::value<std::string>(), "restore a secp256k1 private key")
    ("sign", bp::value<std::string>(), "sign a message")
    ("verify", bp::value<std::string>(), "verify a signature")
    ("encrypt", bp::value<std::string>(), "encrypt a message")
    ("decrypt", bp::value<std::string>(), "decrypt from cipher message");

  backup.add_options()
    ("backup.public-key", bp::value<std::string>(), "public key which is to encrypt a secp256k1 private key");
  restore.add_options()
    ("restore.private-key", bp::value<std::string>(), "sealed private key which is to decrypt a secp256k1 private key");

  sign.add_options()
    ("sign.hex", "message is hex enable")
    ("sign.private-key", bp::value<std::string>(), "sealed private key which is to sign a message");
  verify.add_options()
    ("verify.message", bp::value<std::string>(), "message to be verified")
    ("verify.hex", "message is hex enable")
    ("verify.public-key", bp::value<std::string>(), "public key which is to verify a signature");

  encrypt.add_options()
    ("encrypt.hex", "message is hex enable")
    ("encrypt.public-key", bp::value<std::string>(), "public key which is to encrypt a message");
  decrypt.add_options()
    ("decrypt.hex", "message is hex enable")
    ("decrypt.private-key", bp::value<std::string>(), "sealed private key which is to decrypt a cipher message");
  // clang-format on

  all.add(backup).add(restore).add(encrypt).add(decrypt).add(sign).add(verify);
  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }
  return vm;
}

void create_key(const std::shared_ptr<keymgr_sgx_module> &ptr,
                const std::string &key_dir) {
  ypc::bref public_key, private_key;
  ptr->generate_secp256k1_key_pair(public_key, private_key);
  ypc::bytes pkey(public_key.data(), public_key.len());
  ypc::bytes skey(private_key.data(), private_key.len());
  boost::filesystem::path output_path =
      key_dir / boost::filesystem::path(
                    std::string((const char *)pkey.as<ypc::hex_bytes>().data(),
                                PKEY_FILE_NAME_LENGTH));
  std::string output = output_path.generic_string();
  write_key_pair_to_file(output, pkey, skey);
  std::cout << "Create key pair in file " << output << std::endl;
}

void list_keys(const std::string &key_dir) {
  boost::filesystem::path key_path(key_dir);
  if (!ypc::is_dir_exists(key_dir)) {
    std::stringstream ss;
    ss << "Directory not exist " << key_dir << std::endl;
    throw std::runtime_error(ss.str());
  }
  std::cout << key_dir << std::endl;
  std::cout << std::string(key_dir.size(), '-') << std::endl;
  for (auto &f : boost::make_iterator_range(
           boost::filesystem::directory_iterator(key_path), {})) {
    auto name = f.path().filename().generic_string();
    if (name != "backup") {
      ypc::bytes b_pkey, b_skey;
      read_key_pair_from_file(f.path().generic_string(), b_pkey, b_skey);
      std::cout << name << std::endl;
      std::cout << '\t' << "public key: " << b_pkey << std::endl;
      std::cout << '\t' << "user name : " << std::endl;
      std::cout << '\t' << "create at : " << std::endl;
      std::cout << std::endl;
    }
  }
}

std::pair<ypc::bytes, ypc::bytes>
load_key_pair_from_file(const std::string &key_dir,
                        const std::string &filename) {
  boost::filesystem::path p = key_dir / boost::filesystem::path(filename);
  ypc::bytes b_pkey, b_skey;
  read_key_pair_from_file(p.generic_string(), b_pkey, b_skey);
  return std::make_pair(b_pkey, b_skey);
}

void remove_key(const boost::program_options::variables_map &vm,
                const std::string &key_dir) {
  std::string name = vm["remove"].as<std::string>();
  boost::filesystem::path p = key_dir / boost::filesystem::path(name);
  std::stringstream ss;
  if (!ypc::is_file_exists(p.generic_string())) {
    ss << "File not exist " << p.generic_string() << std::endl;
    throw std::runtime_error(ss.str());
  }
  bool success = boost::filesystem::remove(p);
  if (!success) {
    ss << "Remove file failed " << p.generic_string() << std::endl;
    throw std::runtime_error(ss.str());
  }
  std::cout << "Successfully remove key pair " << name << std::endl;
}

void backup_key(const boost::program_options::variables_map &vm,
                const std::shared_ptr<keymgr_sgx_module> &ptr,
                const std::string &key_dir, const std::string &bak_dir) {
  std::string name = vm["backup"].as<std::string>();
  std::stringstream ss;
  if (!vm.count("backup.public-key")) {
    ss << "`public-key` must be specified!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  boost::filesystem::path p = key_dir / boost::filesystem::path(name);
  if (!ypc::is_file_exists(p.generic_string())) {
    ss << "File not exist " << p.generic_string() << std::endl;
    throw std::runtime_error(ss.str());
  }
  ypc::bytes b_pkey, b_skey;
  read_key_pair_from_file(p.generic_string(), b_pkey, b_skey);

  ypc::bytes public_key =
      ypc::hex_bytes(vm["backup.public-key"].as<std::string>())
          .as<ypc::bytes>();
  ypc::bref backup_key;
  ptr->backup_private_key(b_skey.data(), b_skey.size(), public_key.data(),
                          public_key.size(), backup_key);

  p = bak_dir / boost::filesystem::path(name);
  std::string output = p.generic_string();
  write_key_pair_to_file(output, b_pkey,
                         ypc::bytes(backup_key.data(), backup_key.len()));
  std::cout << "Backup key pair in file " << output << std::endl;
}

void restore_key(const boost::program_options::variables_map &vm,
                 const std::shared_ptr<keymgr_sgx_module> &ptr,
                 const std::string &key_dir, const std::string &bak_dir) {
  std::string name = vm["restore"].as<std::string>();
  std::stringstream ss;
  if (!vm.count("restore.private-key")) {
    ss << "`priavte-key` must be specified!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  boost::filesystem::path p = bak_dir / boost::filesystem::path(name);
  if (!ypc::is_file_exists(p.generic_string())) {
    ss << "File not exist " << p.generic_string() << std::endl;
    throw std::runtime_error(ss.str());
  }
  ypc::bytes b_pkey, b_skey;
  read_key_pair_from_file(p.generic_string(), b_pkey, b_skey);

  ypc::bytes decrypt_skey =
      ypc::hex_bytes(vm["restore.private-key"].as<std::string>())
          .as<ypc::bytes>();
  ypc::bref sealed_key;
  ptr->restore_private_key(b_skey.data(), b_skey.size(), decrypt_skey.data(),
                           decrypt_skey.size(), sealed_key);
  p = key_dir / boost::filesystem::path(name);
  std::string output = p.generic_string();
  write_key_pair_to_file(output, b_pkey,
                         ypc::bytes(sealed_key.data(), sealed_key.len()));
  std::cout << "Restore key pair in file " << output << std::endl;
}

void encrypt_message(const boost::program_options::variables_map &vm,
                     const std::shared_ptr<keymgr_sgx_module> &ptr) {
  std::string msg = vm["encrypt"].as<std::string>();
  if (!vm.count("encrypt.public-key")) {
    std::stringstream ss;
    ss << "`public-key` must be specified!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  ypc::bytes encrypt_pkey =
      ypc::hex_bytes(vm["encrypt.public-key"].as<std::string>())
          .as<ypc::bytes>();
  ypc::bytes b_msg;
  if (vm.count("encrypt.hex")) {
    b_msg = ypc::hex_bytes(msg.c_str()).as<ypc::bytes>();
  } else {
    b_msg = ypc::bytes(msg.c_str());
  }
  ypc::bref cipher;
  // uint32_t cipher_size;
  // uint8_t *cipher;
  ptr->encrypt_message(encrypt_pkey.data(), encrypt_pkey.size(), b_msg.data(),
                       b_msg.size(), cipher);
  std::cout << "Encrypt message \"" << msg << "\", cipher output: "
            << ypc::bytes(cipher.data(), cipher.size());
}

void decrypt_message(const boost::program_options::variables_map &vm,
                     const std::shared_ptr<keymgr_sgx_module> &ptr) {
  ypc::bytes b_cipher =
      ypc::hex_bytes(vm["decrypt"].as<std::string>()).as<ypc::bytes>();
  if (!vm.count("decrypt.private-key")) {
    std::stringstream ss;
    ss << "`private-key` must be specified!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  ypc::bytes decrypt_skey =
      ypc::hex_bytes(vm["decrypt.private-key"].as<std::string>())
          .as<ypc::bytes>();
  ypc::bref msg;
  ptr->decrypt_message(decrypt_skey.data(), decrypt_skey.size(),
                       b_cipher.data(), b_cipher.size(), msg);
  std::cout << "Decrypt cipher \"" << b_cipher << "\", messsage output: "
            << std::string((const char *)msg.data(), msg.size());
}

void sign_message(const boost::program_options::variables_map &vm,
                  const std::shared_ptr<keymgr_sgx_module> &ptr) {
  std::string msg = vm["sign"].as<std::string>();
  if (!vm.count("sign.private-key")) {
    std::stringstream ss;
    ss << "`private-key` must be specified!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  ypc::bytes sign_skey =
      ypc::hex_bytes(vm["sign.private-key"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes b_msg;
  if (vm.count("sign.hex")) {
    b_msg = ypc::hex_bytes(msg.c_str()).as<ypc::bytes>();
  } else {
    b_msg = ypc::bytes(msg.c_str());
  }
  ypc::bref sig;
  ptr->sign_message(sign_skey.data(), sign_skey.size(), b_msg.data(),
                    b_msg.size(), sig);
  std::cout << "Sign message \"" << msg
            << "\", signature output: " << ypc::bytes(sig.data(), sig.size());
}

void verify_signature(const boost::program_options::variables_map &vm,
                      const std::shared_ptr<keymgr_sgx_module> &ptr) {
  ypc::bytes b_sig =
      ypc::hex_bytes(vm["verify"].as<std::string>()).as<ypc::bytes>();
  std::stringstream ss;
  if (!vm.count("verify.public-key")) {
    ss << "`public-key` must be specified!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  ypc::bytes verify_pkey =
      ypc::hex_bytes(vm["verify.public-key"].as<std::string>())
          .as<ypc::bytes>();
  if (!vm.count("verify.message")) {
    ss << "`message` must be specified!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  std::string msg = vm["verify.message"].as<std::string>();
  ypc::bytes b_msg;
  if (vm.count("verify.hex")) {
    b_msg = ypc::hex_bytes(msg.c_str()).as<ypc::bytes>();
  } else {
    b_msg = ypc::bytes(msg.c_str());
  }

  auto ret = ptr->verify_signature(b_msg.data(), b_msg.size(), b_sig.data(),
                                   b_sig.size(), verify_pkey.data(),
                                   verify_pkey.size());
  if (ret) {
    ss << "Signature verify failed! Invalid signature!" << std::endl;
    throw std::runtime_error(ss.str());
  }
  std::cout << "Signature verify success. Valid signature." << std::endl;
}

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cout << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  if (!vm.count("create") && !vm.count("list") && !vm.count("remove") &&
      !vm.count("sign") && !vm.count("verify") && !vm.count("backup") &&
      !vm.count("restore") && !vm.count("encrypt") && !vm.count("decrypt")) {
    std::cout << "one of these options must be specified!" << std::endl;
    return -1;
  }

  auto ptr = std::make_shared<keymgr_sgx_module>("../lib/keymgr.signed.so");
  std::string key_dir = create_dir_if_not_exist(".", ".yeez.key/");
  std::string bak_dir = create_dir_if_not_exist(".yeez.key/", "backup/");

  if (vm.count("create")) {
    create_key(ptr, key_dir);
    return 0;
  }

  if (vm.count("list")) {
    list_keys(key_dir);
    return 0;
  }

  if (vm.count("remove")) {
    remove_key(vm, key_dir);
    return 0;
  }

  if (vm.count("backup")) {
    backup_key(vm, ptr, key_dir, bak_dir);
    return 0;
  }

  if (vm.count("restore")) {
    restore_key(vm, ptr, key_dir, bak_dir);
    return 0;
  }

  if (vm.count("encrypt")) {
    encrypt_message(vm, ptr);
    return 0;
  }

  if (vm.count("decrypt")) {
    decrypt_message(vm, ptr);
    return 0;
  }

  if (vm.count("sign")) {
    sign_message(vm, ptr);
    return 0;
  }

  if (vm.count("verify")) {
    verify_signature(vm, ptr);
    return 0;
  }
  return 0;
}
