#include "cmd_line.h"
#include "ypc/terminus/enclave_interaction.h"

int forward_private_key(ypc::terminus::crypto_pack *crypto,
                        const boost::program_options::variables_map &vm) {
  ypc::bytes private_key = get_param_privatekey(vm);
  ypc::bytes tee_pubkey = get_param_tee_pubkey(vm);
  ypc::bytes enclave_hash = crypto->sha3_256(ypc::bytes("any enclave"));
  if (vm.count("use-enclave-hash")) {
    enclave_hash = ypc::hex_bytes(vm["use-enclave-hash"].as<std::string>())
                       .as<ypc::bytes>();
  }

  auto ei = ypc::terminus::enclave_interaction(crypto);

  auto fi = ei.forward_private_key(private_key, tee_pubkey, enclave_hash);

  std::unordered_map<std::string, ypc::bytes> result;
  result["encrypted_skey"] = fi.encrypted_skey;
  result["forward_sig"] = fi.signature;
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
    for (auto it = result.begin(); it != result.end(); it++) {
      std::cout << it->first << ": " << it->second << std::endl;
    }
  }
  return 0;
}
