#include "common/crypto_prefix.h"
#include "corecommon/nt_cols.h"
#include "ypc/filesystem.h"
#include "ypc/terminus/crypto_pack.h"
#include "ypc/terminus/interaction.h"
#include "ypc/terminus/single_data_onchain_result.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>
#include <ypc/byte.h>

ypc::bytes
get_param_privatekey(const boost::program_options::variables_map &vm) {

  ypc::bytes private_key;
  if (!vm.count("use-privatekey-file") && !vm.count("use-privatekey-hex")) {
    std::cerr << "missing private key, use 'use-privatekey-file' or "
                 "'use-privatekey-hex'"
              << std::endl;
    exit(-1);
  }

  if (vm.count("use-privatekey-hex")) {
    private_key = ypc::hex_bytes(vm["use-privatekey-hex"].as<std::string>())
                      .as<ypc::bytes>();
  } else if (vm.count("use-privatekey-file")) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm["use-privatekey-file"].as<std::string>(), pt);
    private_key = pt.get<ypc::bytes>("private-key");
  }
  return private_key;
}
ypc::bytes
get_param_tee_pubkey(const boost::program_options::variables_map &vm) {
  if (!vm.count("tee-pubkey")) {
    std::cerr << "missing tee-pubkey" << std::endl;
    exit(-1);
  }
  ypc::bytes tee_pubkey =
      ypc::hex_bytes(vm["tee-pubkey"].as<std::string>()).as<ypc::bytes>();
  return tee_pubkey;
}

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Request Generator");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("gen-key", "generate a ECC key pair to encrypt/decrypt request")
    ("encrypt-hex", bp::value<std::string>(), "to encrypt message with hex")
    ("no-password", "no password when gen-key")
    ("forward", "forward private to enclave")
    ("allowance", "generate allowance for param")
    ("dhash", bp::value<std::string>(), "data hash, show data info with hash")
    ("tee-pubkey", bp::value<std::string>(), "TEE public key")
    ("use-privatekey-file", bp::value<std::string>(), "local private key file")
    ("use-privatekey-hex", bp::value<std::string>(), "local private key")
    ("use-publickey-file", bp::value<std::string>(), "local public key file")
    ("use-publickey-hex", bp::value<std::string>(), "local public key")
    ("use-param", bp::value<std::string>(), "param that need to be encrypted")
    ("param-format", bp::value<std::string>(), "[text|hex], default is [hex], param format")
    ("use-enclave-hash", bp::value<std::string>(), "enclave hash")
    ("decrypt-hex", bp::value<std::string>(), "to decrypt message with hex")
    ("sha3-hex", bp::value<std::string>(), "to SHA3-256 message with hex")
    ("output", bp::value<std::string>(), "output result to file with JSON format, only valid for '--use-pubkey' and '--use-param'");

  // clang-format on
  bp::variables_map vm;
  boost::program_options::store(bp::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }

  return vm;
}

int getch() {
  int ch;
  struct termios t_old, t_new;

  tcgetattr(STDIN_FILENO, &t_old);
  t_new = t_old;
  t_new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
  return ch;
}

std::string getpass(const char *prompt, bool show_asterisk = true) {
  const char BACKSPACE = 127;
  const char RETURN = 10;

  std::string password;
  unsigned char ch = 0;

  std::cout << prompt << std::endl;

  while ((ch = getch()) != RETURN) {
    if (ch == BACKSPACE) {
      if (password.length() != 0) {
        if (show_asterisk)
          std::cout << "\b \b";
        password.resize(password.length() - 1);
      }

    } else {
      password += ch;
      if (show_asterisk)
        std::cout << '*';
    }
  }
  std::cout << std::endl;
  return password;
}

int gen_key(ypc::terminus::crypto_pack *crypto,
            const boost::program_options::variables_map &vm) {
  std::string password;
  if (!vm.count("no-password")) {
    password = getpass("Please enter password:", false);
    std::string repass = getpass("Please enter password again: ", false);
    if (password != repass) {
      std::cerr << "Incorrect password. " << std::endl;
      exit(-1);
    }
  }
  auto private_key = crypto->gen_ecc_private_key();
  if (private_key.size() == 0) {
    std::cerr << "failed to generate private key" << std::endl;
    return -1;
  }
  auto public_key = crypto->gen_ecc_public_key_from_private_key(private_key);
  if (public_key.size() == 0) {
    std::cerr << "failed to generate public key " << std::endl;
    return -1;
  }

  if (!vm.count("no-password")) {
    // TODO we should encrypt the private key with password
    std::cerr << "We do not support password mode yet." << std::endl;
    exit(-1);
  }

  if (!vm.count("output")) {
    std::cout << "private key: " << private_key << std::endl;
    std::cout << "public key: " << public_key << std::endl;
  } else {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    pt.put("private-key", private_key);
    pt.put("public-key", public_key);
    boost::property_tree::json_parser::write_json(output_path, pt);
  }
  return 0;
}

int decrypt_message(ypc::terminus::crypto_pack *crypto,
                    const boost::program_options::variables_map &vm) {
  ypc::bytes message =
      ypc::hex_bytes(vm["decrypt-hex"].as<std::string>()).as<ypc::bytes>();

  ypc::bytes private_key = get_param_privatekey(vm);

  ypc::bytes data = crypto->ecc_decrypt(message, private_key,
                                        ypc::utc::crypto_prefix_arbitrary);
  if (data.size() == 0) {
    std::cerr << "failed to decrypt data" << std::endl;
    exit(-1);
  }

  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());
    std::ofstream os(output_path, std::ios::out | std::ios::binary);
    os.write((const char *)data.data(), data.size());
  } else {
    std::cout << data << std::endl;
  }

  return 0;
}

int encrypt_message(ypc::terminus::crypto_pack *crypto,
                    const boost::program_options::variables_map &vm) {
  ypc::bytes public_key;
  if (!vm.count("use-publickey-file") && !vm.count("use-publickey-hex")) {
    std::cerr << "missing public key, use 'use-publickey-file' or "
                 "'use-publickey-hex'"
              << std::endl;
    exit(-1);
  }
  if (vm.count("use-publickey-hex")) {
    public_key = ypc::hex_bytes(vm["use-publickey-hex"].as<std::string>())
                     .as<ypc::bytes>();
  } else if (vm.count("use-publickey-file")) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm["use-publickey-file"].as<std::string>(), pt);
    public_key = pt.get<ypc::bytes>("public-key");
    std::cout << "public key: " << public_key << std::endl;
  }

  ypc::bytes message =
      ypc::hex_bytes(vm["encrypt-hex"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes data = crypto->ecc_encrypt(message, public_key,
                                        ypc::utc::crypto_prefix_arbitrary);
  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());
    std::ofstream os(output_path, std::ios::out);
    os << data;
    // os.write((const char *)data.data(), data.size());
  } else {
    std::cout << data << std::endl;
  }
  return 0;
}

int sha3_message(ypc::terminus::crypto_pack *crypto,
                 const boost::program_options::variables_map &vm) {

  ypc::bytes message =
      ypc::hex_bytes(vm["sha3-hex"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes data = crypto->sha3_256(message);
  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());
    std::ofstream os(output_path, std::ios::out);
    os << data;
    // os.write((const char *)data.data(), data.size());
  } else {
    std::cout << data << std::endl;
  }
  return 0;
}
uint32_t forward_private_key(ypc::terminus::crypto_pack *crypto,
                             const boost::program_options::variables_map &vm) {
  ypc::bytes private_key = get_param_privatekey(vm);
  ypc::bytes tee_pubkey = get_param_tee_pubkey(vm);
  ypc::bytes enclave_hash = crypto->sha3_256(ypc::bytes("any enclave"));
  if (vm.count("use-enclave-hash")) {
    enclave_hash = ypc::hex_bytes(vm["use-enclave-hash"].as<std::string>())
                       .as<ypc::bytes>();
  }

  ypc::bytes encrypted_skey = crypto->ecc_encrypt(
      private_key, tee_pubkey, ypc::utc::crypto_prefix_forward);
  ypc::bytes to_sign_msg = private_key + tee_pubkey + enclave_hash;
  ypc::bytes sig = crypto->sign_message(to_sign_msg, private_key);

  std::unordered_map<std::string, ypc::bytes> result;
  result["encrypted_skey"] = encrypted_skey;
  result["forward_sig"] = sig;
  result["enclave_hash"] = enclave_hash;

  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    for (auto it = result.begin(); it != result.end(); it++) {
      pt.put(it->first, it->second);
    }
    boost::property_tree::json_parser::write_json(output_path, pt);
  } else {
    std::cout << "encrypted skey: " << encrypted_skey << std::endl;
    std::cout << "forward sig: " << sig << std::endl;
    std::cout << "enclave hash: " << enclave_hash << std::endl;
  }
  return 0;
}
uint32_t generate_allowance(ypc::terminus::crypto_pack *crypto,
                            const boost::program_options::variables_map &vm) {
  ypc::bytes private_key = get_param_privatekey(vm);

  std::string format = "hex";
  ypc::bytes param;
  if (vm.count("param-format")) {
    format = vm["param-format"].as<std::string>();
  }
  if (format == "hex") {
    param = ypc::hex_bytes(vm["use-param"].as<std::string>()).as<ypc::bytes>();
  } else if (format == "text") {
    param = ypc::bytes(vm["use-param"].as<std::string>());
  } else {
    std::cerr << "unknow format from '--param-format='" << format << std::endl;
    exit(-1);
  }
  ypc::bytes hash = param;
  ypc::bytes sig = crypto->sign_message(hash, private_key);
  ypc::bytes pkey = crypto->gen_ecc_public_key_from_private_key(private_key);
  ypc::bytes allowance =
      crypto->ecc_encrypt(sig, pkey, ypc::utc::crypto_prefix_arbitrary);

  std::unordered_map<std::string, ypc::bytes> result;
  result["encrypted_sig"] = allowance;
  result["pkey"] = pkey;

  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    for (auto it = result.begin(); it != result.end(); it++) {
      pt.put(it->first, it->second);
    }
    boost::property_tree::json_parser::write_json(output_path, pt);
  } else {
    std::cout << "encrypted sig: " << allowance << std::endl;
    std::cout << "pkey: " << pkey << std::endl;
  }
  return 0;
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

  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();

  if (vm.count("gen-key")) {
    return gen_key(crypto.get(), vm);
  }

  if (vm.count("decrypt-hex")) {
    return decrypt_message(crypto.get(), vm);
  }
  if (vm.count("encrypt-hex")) {
    return encrypt_message(crypto.get(), vm);
  }
  if (vm.count("forward")) {
    return forward_private_key(crypto.get(), vm);
  }
  if (vm.count("allowance")) {
    return generate_allowance(crypto.get(), vm);
  }
  if (vm.count("sha3-hex")) {
    return sha3_message(crypto.get(), vm);
  }

  if (!vm.count("dhash")) {
    std::cout << "No data hash is provided!" << std::endl;
    return -1;
  }

  ypc::bytes dhash =
      ypc::hex_bytes(vm["dhash"].as<std::string>()).as<ypc::bytes>();

  ypc::bytes tee_pubkey = get_param_tee_pubkey(vm);

  ypc::bytes private_key = get_param_privatekey(vm);

  std::unordered_map<std::string, ypc::bytes> result;
  result["data-hash"] = dhash;
  result["provider-pkey"] = tee_pubkey;

  std::string format = "hex";
  ypc::bytes param;
  if (vm.count("param-format")) {
    format = vm["param-format"].as<std::string>();
  }
  if (format == "hex") {
    param = ypc::hex_bytes(vm["use-param"].as<std::string>()).as<ypc::bytes>();
  } else if (format == "text") {
    param = ypc::bytes(vm["use-param"].as<std::string>());
  } else {
    std::cout << "unknow format from '--param-format='" << format << std::endl;
    return -1;
  }

  ypc::terminus::single_data_onchain_result std_interaction(crypto.get());

  auto pubkey = crypto->gen_ecc_public_key_from_private_key(private_key);

  result["analyzer-pkey"] = pubkey;

  ypc::bytes enclave_hash;
  if (vm.count("use-enclave-hash")) {
    enclave_hash = ypc::hex_bytes(vm["use-enclave-hash"].as<std::string>())
                       .as<ypc::bytes>();
  } else {
    std::cerr << "missing enclave, use 'use-enclave-hash' " << std::endl;
    exit(-1);
  }

  result["program-enclave-hash"] =
      ypc::bytes(enclave_hash.data(), enclave_hash.size());

  auto request = std_interaction.generate_request(param, tee_pubkey, dhash,
                                                  enclave_hash, private_key);
  if (request.encrypted_param.size() == 0) {
    std::cerr << "failed to encrypt param" << std::endl;
    return -1;
  }
  if (request.signature.size() == 0) {
    std::cerr << "failed to generate signature" << std::endl;
    return -1;
  }

  result["encrypted-input"] = request.encrypted_param;
  result["forward-sig"] = request.signature;
  result["encrypted-skey"] = request.encrypted_skey;

  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    for (auto it = result.begin(); it != result.end(); it++) {
      pt.put(it->first, it->second);
    }
    boost::property_tree::json_parser::write_json(output_path, pt);
  }
  return 0;
}
