#include "cmd_line.h"

int generate_request(ypc::terminus::crypto_pack *crypto,
                     const boost::program_options::variables_map &vm) {
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

  ypc::bytes param = get_param_use_param(vm);

  ypc::terminus::single_data_onchain_result std_interaction(crypto);

  auto pubkey = crypto->gen_ecc_public_key_from_private_key(private_key);

  result["analyzer-pkey"] = pubkey;

  ypc::bytes enclave_hash =
      ypc::hex_bytes(vm["use-enclave-hash"].as<std::string>()).as<ypc::bytes>();

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
