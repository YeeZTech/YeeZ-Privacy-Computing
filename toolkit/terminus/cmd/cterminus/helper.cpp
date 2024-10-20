#include "cmd_line.h"

ypc::bytes
get_param_privatekey(const boost::program_options::variables_map &vm) {

  ypc::bytes private_key;
  ypc::po(vm).require(ypc::opt("use-privatekey-file") ||
                      ypc::opt("use-privatekey-hex"));

  if (vm.count("use-privatekey-hex") != 0u) {
    private_key = ypc::hex_bytes(vm["use-privatekey-hex"].as<std::string>())
                      .as<ypc::bytes>();
  } else if (vm.count("use-privatekey-file") != 0u) {
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

  if (vm.count("use-publickey-hex") != 0u) {
    private_key = ypc::hex_bytes(vm["use-publickey-hex"].as<std::string>())
                      .as<ypc::bytes>();
  } else if (vm.count("use-publickey-file") != 0u) {
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
  if (vm.count("param-format") != 0u) {
    format = vm["param-format"].as<std::string>();
  }
  std::string param_content;
  if (vm.count("use-param") != 0u) {
    param_content = vm["use-param"].as<std::string>();
  } else if (vm.count("use-param-file") != 0u) {
    std::ifstream ifs(vm["use-param-file"].as<std::string>());
    if (!ifs.is_open()) {
      std::cerr << "file open failed!" << std::endl;
      exit(-1);
    }
    ifs.seekg(0, ifs.end);
    auto size = ifs.tellg();
    param_content = std::string(size, '0');
    ifs.seekg(0, ifs.beg);
    ifs.read((char *)param_content.c_str(), size);
    ifs.close();
  }
  if (format == "hex") {
    param = ypc::hex_bytes(param_content).as<ypc::bytes>();
  } else if (format == "text") {
    param = ypc::bytes(param_content);
  } else {
    std::cerr << "unknow format from '--param-format='" << format << std::endl;
    exit(-1);
  }
  return param;
}

