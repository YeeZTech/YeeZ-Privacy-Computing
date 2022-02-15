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
#include <ypc/sgx/parser_sgx_module.h>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Request Generator");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("enclave-type", bp::value<std::string>(), "enclave type, can be [keymgr | parser | datahub], default is [parser]")
    ("enclave", bp::value<std::string>(), "enclave file path")
    ("output", bp::value<std::string>(), "output result to file with JSON format");

  // clang-format on
  bp::variables_map vm;
  boost::program_options::store(bp::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }

  return vm;
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

  std::unordered_map<std::string, ypc::bytes> result;

  ypc::bytes enclave_hash;
  if (etype == "parser") {
    ypc::parser_sgx_module mod(enclave_path.c_str());
    mod.get_enclave_hash(enclave_hash);
    // enclave_hash = ypc::bytes(_enclave_hash.data(), _enclave_hash.size());
  }
  // TODO we should support other types, and more info here, like version,
  // signer

  result["enclave-hash"] = enclave_hash;

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
