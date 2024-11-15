#include "iodef.h"
#include "ypc/core/configuration.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/version.h"
#include "ypc/stbox/stx_status.h"
#include "dianshu_parser.h"
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
    ("module-lib", bp::value<std::string>(), "module lib path")
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
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }
  if (vm.count("module-lib") == 0u) {
    std::cerr << "module-lib not specified" << std::endl;
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
  std::string module_lib = vm["module-lib"].as<std::string>();
  auto g_parser = dianshu_parser::GetParser();
  g_parser->Init(input_param, module_lib);
  std::cout << "start to parse" << std::endl;
  g_parser->parse();

  std::string output_fp = vm["output"].as<std::string>();
  try {
    std::ofstream os(output_fp, std::ios::out | std::ios::binary);
    const std::string &res = g_parser->get_result_str();
    os.write(res.data(), res.size());
  } catch (const std::exception &e) {
    std::cerr << "cannot open " << output_fp << std::endl;
    return 1;
  }
  return 0;
}


extern "C" {
uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len);
}

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len) {
  auto g_parser = dianshu_parser::GetParser();
  return g_parser->next_data_batch(data_hash, hash_size, data, len);
}
