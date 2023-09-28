#include "iodef.h"
#include "ypc/core/version.h"
#include "sgx_bridge.h"
#include "ypc/core/configuration.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/oram_sealed_file.h"
#include "ypc/stbox/stx_status.h"
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using stx_status = stbox::stx_status;
using namespace ypc;

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Analyzer options");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("version", "show version")
    ("input", bp::value<std::string>(), "input parameters JSON file")
    ("output", bp::value<std::string>(), "output result JSON file")
    ("gen-example-input", bp::value<std::string>(), "generate example input parameters JSON file");
  // clang-format on

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help") != 0u) {
    std::cout << all << std::endl;
    exit(-1);
  }
  if (vm.count("version") != 0u) {
    std::cout << ypc::get_ypc_version() << std::endl;
    exit(-1);
  }
  if (vm.count("gen-example-input") != 0u) {
    input_param_t example;
    ypc::ntjson::to_json_file(example,
                              vm["gen-example-input"].as<std::string>());
    exit(-1);
  }
  return vm;
}

int main(int argc, char *argv[]) {

  // google::InitGoogleLogging(argv[0]);
  // google::InstallFailureSignalHandler();

  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }
  if (vm.count("input") == 0u) {
    std::cerr << "input not specified" << std::endl;
    return -1;
  }

  if (vm.count("output") == 0u) {
    std::cerr << "output not specified" << std::endl;
    return -1;
  }

  input_param_t input_param =
      ypc::ntjson::from_json_file<input_param_t>(vm["input"].as<std::string>());

  o_parser = std::make_shared<oram_parser>(input_param);
  std::cout << "start to parse" << std::endl;
  o_parser->oram_parse();

  std::string output_fp = vm["output"].as<std::string>();
  try {
    std::ofstream os(output_fp, std::ios::out | std::ios::binary);
    const std::string &res = o_parser->get_result_str();
    os.write(res.data(), res.size());
  } catch (const std::exception &e) {
    std::cerr << "cannot open " << output_fp << std::endl;
    return 1;
  }

  return 0;
}

