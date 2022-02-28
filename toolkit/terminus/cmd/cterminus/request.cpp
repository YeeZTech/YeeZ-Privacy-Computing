#include "cmd_line.h"

int generate_request(ypc::terminus::crypto_pack *crypto,
                     const boost::program_options::variables_map &vm) {
  ypc::bytes pubkey = get_param_publickey(vm);

  std::unordered_map<std::string, ypc::bytes> result;

  ypc::bytes param = get_param_use_param(vm);

  ypc::terminus::single_data_onchain_result std_interaction(crypto);

  result["analyzer-pkey"] = pubkey;

  auto request = std_interaction.generate_request(param, pubkey);
  if (request.size() == 0) {
    std::cerr << "failed to encrypt param" << std::endl;
    return -1;
  }

  result["encrypted-input"] = request;

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
