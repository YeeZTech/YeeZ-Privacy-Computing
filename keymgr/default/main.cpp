#include "keymgr/common/util.h"
#include "keymgr_sgx_module.h"
#include "stbox/stx_common.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

using namespace stbox;
boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Key Manager options");
  bp::options_description backup("YeeZ Backup options");
  bp::options_description restore("YeeZ Restore options");
  bp::options_description encrypt("YeeZ Encrypt options");
  bp::options_description decrypt("YeeZ Decrypt options");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("create", "create a secp256k1 key pair")
    ("list", "list secp256k1 keys")
    ("remove", bp::value<std::string>(), "remove a secp256k1 key pair")
    ("backup", bp::value<std::string>(), "backup a secp256k1 private key")
    ("restore", bp::value<std::string>(), "restore a secp256k1 private key")
    ("encrypt", bp::value<std::string>(), "encrypt a message")
    ("decrypt", bp::value<std::string>(), "decrypt from cipher message");

  backup.add_options()
    ("backup.public-key", bp::value<std::string>(), "public key which is to encrypt a secp256k1 private key");
  restore.add_options()
    ("restore.private-key", bp::value<std::string>(), "sealed private key which is to decrypt a secp256k1 private key");

  encrypt.add_options()
    ("encrypt.hex", "message is hex enable")
    ("encrypt.public-key", bp::value<std::string>(), "public key which is to encrypt a message");
  decrypt.add_options()
    ("decrypt.hex", "message is hex enable")
    ("decrypt.private-key", bp::value<std::string>(), "sealed private key which is to decrypt a cipher message");
  // clang-format on

  all.add(backup).add(restore).add(encrypt).add(decrypt);
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
  } catch (...) {
    std::cout << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  if (!vm.count("create") && !vm.count("list") && !vm.count("remove") &&
      !vm.count("backup") && !vm.count("restore") && !vm.count("encrypt") &&
      !vm.count("decrypt")) {
    std::cout << "one of these options must be specified!" << std::endl;
    return -1;
  }

  auto ptr = std::make_shared<keymgr_sgx_module>("../lib/keymgr.signed.so");
  std::string key_dir = create_dir_if_not_exist(".", ".yeez.key/");
  std::string bak_dir = create_dir_if_not_exist(".yeez.key/", "backup/");

  if (vm.count("create")) {
    ypc::bref public_key, private_key;
    ptr->generate_secp256k1_key_pair(public_key, private_key);
    ypc::bytes pkey(public_key.data(), public_key.len());
    ypc::bytes skey(private_key.data(), private_key.len());
    boost::filesystem::path output_path =
        key_dir / boost::filesystem::path(std::string(
                      (const char *)pkey.as<ypc::hex_bytes>().data(),
                      PKEY_FILE_NAME_LENGTH));
    std::string output = output_path.generic_string();
    write_key_pair_to_file(output, pkey, skey);
    std::cout << "Create key pair in file " << output << std::endl;
  } else if (vm.count("list")) {
    boost::filesystem::path key_path(key_dir);
    if (!ypc::is_dir_exists(key_dir)) {
      std::cout << "Directory not exist " << key_dir << std::endl;
      return -1;
    }
    for (auto &f : boost::make_iterator_range(
             boost::filesystem::directory_iterator(key_path), {})) {
      auto name = f.path().filename().generic_string();
      if (name != "backup") {
        std::cout << name << std::endl;
      }
    }
  } else if (vm.count("remove")) {
    std::string name = vm["remove"].as<std::string>();
    boost::filesystem::path p = key_dir / boost::filesystem::path(name);
    if (!ypc::is_file_exists(p.generic_string())) {
      std::cout << "File not exist " << p.generic_string() << std::endl;
      return -1;
    }
    bool success = boost::filesystem::remove(p);
    if (!success) {
      std::cout << "Remove file failed " << p.generic_string() << std::endl;
      return -1;
    }
    std::cout << "Successfully remove key pair " << name << std::endl;
  } else if (vm.count("backup")) {
    std::string name = vm["backup"].as<std::string>();
    if (!vm.count("backup.public-key")) {
      std::cout << "`public-key` must be specified!" << std::endl;
      return -1;
    }
    boost::filesystem::path p = key_dir / boost::filesystem::path(name);
    if (!ypc::is_file_exists(p.generic_string())) {
      std::cout << "File not exist " << p.generic_string() << std::endl;
      return -1;
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
  } else if (vm.count("restore")) {
    std::string name = vm["restore"].as<std::string>();
    if (!vm.count("restore.private-key")) {
      std::cout << "`priavte-key` must be specified!" << std::endl;
      return -1;
    }
    boost::filesystem::path p = bak_dir / boost::filesystem::path(name);
    if (!ypc::is_file_exists(p.generic_string())) {
      std::cout << "File not exist " << p.generic_string() << std::endl;
      return -1;
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
  } else if (vm.count("encrypt")) {
    std::string msg = vm["encrypt"].as<std::string>();
    if (!vm.count("encrypt.public-key")) {
      std::cout << "`public-key` must be specified!" << std::endl;
      return -1;
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
  } else if (vm.count("decrypt")) {
    ypc::bytes b_cipher =
        ypc::hex_bytes(vm["decrypt"].as<std::string>()).as<ypc::bytes>();
    if (!vm.count("decrypt.private-key")) {
      std::cout << "`private-key` must be specified!" << std::endl;
      return -1;
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
  return 0;
}
