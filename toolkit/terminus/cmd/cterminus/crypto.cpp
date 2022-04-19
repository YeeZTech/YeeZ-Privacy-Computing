#include "cmd_line.h"

int decrypt_message(ypc::terminus::crypto_pack *crypto,
                    const boost::program_options::variables_map &vm) {
  ypc::bytes message = get_param_use_param(vm);

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
  ypc::bytes public_key = get_param_publickey(vm);

  ypc::bytes message = get_param_use_param(vm);
  ypc::bytes data = crypto->ecc_encrypt(message, public_key,
                                        ypc::utc::crypto_prefix_arbitrary);
  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());
    std::ofstream os(output_path, std::ios::out);
    os << data;
  } else {
    std::cout << data << std::endl;
  }
  return 0;
}

int sha3_message(ypc::terminus::crypto_pack *crypto,
                 const boost::program_options::variables_map &vm) {

  ypc::bytes message = get_param_use_param(vm);
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
