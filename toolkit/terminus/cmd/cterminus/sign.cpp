#include "cmd_line.h"
#include "ypc/terminus/enclave_interaction.h"

int sign_message(ypc::terminus::crypto_pack *crypto,
                        const boost::program_options::variables_map &vm) {
  ypc::bytes message = get_param_use_param(vm);
  ypc::bytes private_key = get_param_privatekey(vm);
  ypc::bytes signature = crypto->sign_message(message, private_key);

  std::cout << "message " << message << std::endl;
  std::cout << "signature " << signature << std::endl;

  std::unordered_map<std::string, ypc::bytes> result;
  result["message"] = message;
  result["signature"] = signature;
  if (vm.count("output") != 0u) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    for (auto &it : result) {
      pt.put(it.first, it.second);
    }
    boost::property_tree::json_parser::write_json(output_path, pt);
  }

  return 0;
}

int verify_signature(ypc::terminus::crypto_pack *crypto,
                     const boost::program_options::variables_map &vm) {
  ypc::bytes message = get_param_use_param(vm);
  ypc::bytes public_key = get_param_publickey(vm);
  ypc::bytes signature =
      ypc::hex_bytes(vm["use-signature"].as<std::string>()).as<ypc::bytes>();
  bool valid = crypto->verify_message_signature(signature, message, public_key);
  if (vm.count("output") != 0u) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    pt.put("verify_result", valid);
    boost::property_tree::json_parser::write_json(output_path, pt);
  }
  return 0;
}
