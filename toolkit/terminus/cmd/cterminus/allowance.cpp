#include "cmd_line.h"

int generate_allowance(ypc::terminus::crypto_pack *crypto,
                       const boost::program_options::variables_map &vm) {
  ypc::bytes private_key = get_param_privatekey(vm);

  ypc::bytes hash =
      ypc::hex_bytes(vm["use-param"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes enclave_hash =
      ypc::hex_bytes(vm["use-enclave-hash"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes dian_pkey =
      ypc::hex_bytes(vm["tee-pubkey"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes dhash =
      ypc::hex_bytes(vm["dhash"].as<std::string>()).as<ypc::bytes>();

  ypc::bytes to_sig_data = hash + enclave_hash + dian_pkey + dhash;
  ypc::bytes sig = crypto->sign_message(to_sig_data, private_key);
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
