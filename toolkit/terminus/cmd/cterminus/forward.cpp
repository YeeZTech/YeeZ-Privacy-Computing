#include "cmd_line.h"

int forward_private_key(ypc::terminus::crypto_pack *crypto,
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
