#include "keymgr/common/util.h"
#include "keymgr_sgx_module.h"
#include "stbox/stx_common.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/range/iterator_range.hpp>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>

using namespace stbox;
using ntt = ypc::nt<ypc::bytes>;
bool is_user_id_valid(const std::string &user_id) {
  std::string blacklists("!#$%^&*()-+=\\|`{}[]:;\"<>'?/");
  for (auto c : blacklists) {
    if (user_id.find(c) != std::string::npos) {
      std::cerr << "can't have " << blacklists << ", yet found " << c
                << std::endl;
      return false;
    }
  }
  return true;
}

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Key Manager options");
  bp::options_description encrypt("YeeZ Encrypt options");
  bp::options_description decrypt("YeeZ Decrypt options");
  bp::options_description sign("YeeZ Sign options");
  bp::options_description verify("YeeZ Verify options");
  bp::options_description create("YeeZ Create Key options");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("create", "create a secp256k1 key pair")
    ("list", "list secp256k1 keys")
    ("remove", bp::value<std::string>(), "remove a secp256k1 key pair")
    ("sign", bp::value<std::string>(), "sign a message")
    ("verify", bp::value<std::string>(), "verify a signature")
    ("encrypt", bp::value<std::string>(), "encrypt a message")
    ("decrypt", bp::value<std::string>(), "decrypt from cipher message");

  create.add_options()
    ("user-id", bp::value<std::string>(), "user id for created key");

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

  all.add(create).add(encrypt).add(decrypt).add(sign).add(verify);
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
                const std::string &key_dir,
                const boost::program_options::variables_map &vm) {
  ypc::bref public_key, private_key;
  ptr->generate_secp256k1_key_pair(public_key, private_key);
  ypc::bytes pkey(public_key.data(), public_key.len());
  ypc::bytes skey(private_key.data(), private_key.len());
  boost::filesystem::path output_path =
      key_dir / boost::filesystem::path(
                    std::string((const char *)pkey.as<ypc::hex_bytes>().data(),
                                PKEY_FILE_NAME_LENGTH));
  std::string output = output_path.generic_string();
  const auto now = std::chrono::system_clock::now();

  uint64_t s =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
          .count();
  ntt::keymgr_key_package_t key;
  key.set<ntt::pkey, ntt::sealed_skey, ntt::timestamp>(pkey, skey, s);
  std::string userid;
  if (vm.count("user-id")) {
    userid = vm["user-id"].as<std::string>();
  } else {
    std::cout << "Input user id: ";
    std::cin >> userid;
  }

  bool v = is_user_id_valid(userid);
  if (!v) {
    return;
  }

  // TODO check userid
  key.set<ntt::user_id>(userid);
  write_key_pair_to_file(output, key);
  std::cout << "Create key pair in file " << output << std::endl;
}

void list_keys(const std::string &key_dir,
               const std::shared_ptr<keymgr_sgx_module> &ptr) {
  boost::filesystem::path key_path(key_dir);
  if (!ypc::is_dir_exists(key_dir)) {
    std::stringstream ss;
    ss << "Directory not exist " << key_dir << std::endl;
    throw std::runtime_error(ss.str());
  }
  std::cout << "listing keys in " << key_dir << std::endl;
  std::cout << std::string(key_dir.size() + 18, '-') << std::endl;
  uint counter = 0;
  for (auto &f : boost::make_iterator_range(
           boost::filesystem::directory_iterator(key_path), {})) {
    auto name = f.path().filename().generic_string();
    if (name != "backup") {
      counter++;
      ntt::keymgr_key_package_t key;
      read_key_pair_from_file(f.path().generic_string(), key);
      std::cout << ">> key " << counter << ": " << name << std::endl;
      std::cout << '\t' << "public key: " << key.get<ntt::pkey>() << std::endl;
      std::cout << '\t' << "user id: " << key.get<ntt::user_id>() << std::endl;
      uint64_t s = key.get<ntt::timestamp>();
      char *dt = ctime((time_t *)&s);
      std::cout << '\t' << "create at: " << dt;

      { // testing aviability
        ypc::bytes encrypt_pkey = key.get<ntt::pkey>();
        ypc::bytes b_msg("hello world");
        ypc::bref cipher;
        uint32_t ret =
            ptr->encrypt_message(encrypt_pkey.data(), encrypt_pkey.size(),
                                 b_msg.data(), b_msg.size(), cipher);
        if (!ret) {
          std::cout << "\t[\033[1;32m"
                    << "Check encrypt success"
                    << "\033[0m"
                    << "]" << std::endl;
        } else {
          std::cout << "\t[\033[1;31m"
                    << "Check encrypt failed: " << stbox::status_string(ret)
                    << "\033[0m"
                    << "]" << std::endl;
        }
        ypc::bytes sealed_skey = key.get<ntt::sealed_skey>();
        ypc::bref raw_msg;
        ret = ptr->decrypt_message(sealed_skey.data(), sealed_skey.size(),
                                   cipher.data(), cipher.size(), raw_msg);
        if (!ret) {
          std::cout << "\t[\033[1;32m"
                    << "Check decrypt success"
                    << "\033[0m"
                    << "]" << std::endl;
        } else {
          std::cout << "\t[\033[1;31m"
                    << "Check decrypt failed: " << stbox::status_string(ret)
                    << "\033[0m"
                    << "]" << std::endl;
        }
      }

      std::cout << std::endl;
    }
  }
  if (counter == 0) {
    std::cout << "Cannot find any key pairs. You may create one by yourself."
              << std::endl;
  }
}

std::pair<ypc::bytes, ypc::bytes>
load_key_pair_from_file(const std::string &key_dir,
                        const std::string &filename) {
  boost::filesystem::path p = key_dir / boost::filesystem::path(filename);
  ntt::keymgr_key_package_t key;
  read_key_pair_from_file(p.generic_string(), key);
  ypc::bytes b_pkey = key.get<ntt::pkey>();
  ypc::bytes b_skey = key.get<ntt::sealed_skey>();
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

std::string find_keymgr_enclave_path(const std::string &current_path) {
  // 1. check if keymgr.signed.so in project lib dir
  std::string project_lib_path =
      ypc::join_path(ypc::dirname(current_path), "../lib/keymgr.signed.so");
  if (ypc::is_file_exists(project_lib_path)) {
    return project_lib_path;
  }
  // 2. check if exists in /usr/local/lib
  std::string usr_local_lib_path =
      ypc::join_path("/usr/local/lib", "./keymgr.signed.so");
  if (ypc::is_file_exists(usr_local_lib_path)) {
    return usr_local_lib_path;
  }
  // 3. check if exists in /usr/lib
  std::string usr_lib_path = ypc::join_path("/usr/lib", "./keymgr.signed.so");
  if (ypc::is_file_exists(usr_lib_path)) {
    return usr_lib_path;
  }
  throw std::runtime_error("cannot find keymgr.signed.so file!");
}

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  if (!vm.count("create") && !vm.count("list") && !vm.count("remove") &&
      !vm.count("sign") && !vm.count("verify") && !vm.count("encrypt") &&
      !vm.count("decrypt")) {
    std::cerr << "one of [create, list, remove, sign, verify, encrypt, "
                 "decrypt] must be specified!"
              << std::endl;
    return -1;
  }

  std::string kmgr_enclave_path =
      find_keymgr_enclave_path(ypc::complete_path(argv[0]));
  std::shared_ptr<keymgr_sgx_module> ptr;
  try {
    ptr = std::make_shared<keymgr_sgx_module>(kmgr_enclave_path.c_str());
  } catch (const std::exception &e) {
    std::cerr << "cannot open enclave file " << kmgr_enclave_path << ", "
              << e.what();
    return -1;
  }

  std::string key_dir = create_dir_if_not_exist(".", ".yeez.key/");
  std::string bak_dir = create_dir_if_not_exist(".yeez.key/", "backup/");

  if (vm.count("create")) {
    create_key(ptr, key_dir, vm);
    return 0;
  }

  if (vm.count("list")) {
    list_keys(key_dir, ptr);
    return 0;
  }

  if (vm.count("remove")) {
    remove_key(vm, key_dir);
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
