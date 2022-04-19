#include "cmd_line.h"

ypc::bytes
get_param_privatekey(const boost::program_options::variables_map &vm) {

  ypc::bytes private_key;
  ypc::po(vm).require(ypc::opt("use-privatekey-file") ||
                      ypc::opt("use-privatekey-hex"));

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
get_param_publickey(const boost::program_options::variables_map &vm) {

  ypc::bytes private_key;
  ypc::po(vm).require(ypc::opt("use-publickey-file") ||
                      ypc::opt("use-publickey-hex"));

  if (vm.count("use-publickey-hex")) {
    private_key = ypc::hex_bytes(vm["use-publickey-hex"].as<std::string>())
                      .as<ypc::bytes>();
  } else if (vm.count("use-publickey-file")) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm["use-publickey-file"].as<std::string>(), pt);
    private_key = pt.get<ypc::bytes>("public-key");
  }
  return private_key;
}

ypc::bytes
get_param_tee_pubkey(const boost::program_options::variables_map &vm) {
  ypc::po(vm).require("tee-pubkey");
  ypc::bytes tee_pubkey =
      ypc::hex_bytes(vm["tee-pubkey"].as<std::string>()).as<ypc::bytes>();
  return tee_pubkey;
}

ypc::bytes
get_param_use_param(const boost::program_options::variables_map &vm) {
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
    exit(-1);
  }
  return param;
}

