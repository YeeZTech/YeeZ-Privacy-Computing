#include "ypc/filesystem.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>
#include <keymgr/common/util.h>
#include <keymgr/default/keymgr_sgx_module.h>
#include <stbox/ebyte.h>
#include <unordered_map>
#include <ypc/byte.h>
#include <ypc/version.h>
#include <ypc/sgx/parser_sgx_module.h>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Enclave Hash Dumper");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("version", "show version")
    ("enclave-type", bp::value<std::string>(), "enclave type, can be [keymgr | parser], default is [parser]")
    ("enclave", bp::value<std::string>(), "enclave file path")
    ("output", bp::value<std::string>(), "output result to file with JSON format");

  // clang-format on
  bp::variables_map vm;
  boost::program_options::store(bp::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }

  if (vm.count("version")) {
    std::cout << ypc::get_ypc_version() << std::endl;
    exit(-1);
  }

  return vm;
}

void get_enclave_type(uint32_t parser_type, std::unordered_map<std::string, std::string> &result) {
  result["result-type"] = std::to_string(parser_type & 0xf);
  parser_type >>= 4;
  result["data-source-type"] = std::to_string(parser_type & 0xf);
  parser_type >>= 4;
  result["has-model"] = std::to_string(parser_type & 0x1);
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

  std::string etype = "parser";
  if (vm.count("enclave-type")) {
    etype = vm["enclave-type"].as<std::string>();
  }

  std::string enclave_path;
  if (vm.count("enclave")) {
    enclave_path = vm["enclave"].as<std::string>();
  } else {
    std::cerr << "missing --enclave";
    exit(-1);
  }

  std::unordered_map<std::string, std::string> result;

  ypc::bytes enclave_hash;
  if (etype == "parser") {
    ypc::parser_sgx_module mod(enclave_path.c_str());
    mod.get_enclave_hash(enclave_hash);

    uint32_t parser_type = mod.get_parser_type();
    get_enclave_type(parser_type, result);
    // enclave_hash = ypc::bytes(_enclave_hash.data(), _enclave_hash.size());
  }
  // TODO we should support other types, and more info here, like version,
  // signer

  std::stringstream ss;
  ss << enclave_hash.as<ypc::hex_bytes>();
  result["enclave-hash"] = ss.str();
  result["version"] = ypc::get_ypc_version();

  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    boost::property_tree::ptree pt;
    for (auto it = result.begin(); it != result.end(); it++) {
      pt.put(it->first, it->second);
    }
    boost::property_tree::json_parser::write_json(output_path, pt);
  } else {
    for (auto it = result.begin(); it != result.end(); ++it) {
      std::cout << it->first << ": " << it->second << std::endl;
    }
  }

  return 0;
}
