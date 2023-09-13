#ifndef YEEZ_PRIVACY_COMPUTING_ANALYZER_H
#define YEEZ_PRIVACY_COMPUTING_ANALYZER_H

#include "iodef.h"
#include "ypc/core/version.h"
#include "sgx_bridge.h"
#include "ypc/core/configuration.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/sealed_file.h"
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
    ("output", bp::value<std::string>(), "output result JSON file");
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
  return vm;
}

#endif //YEEZ_PRIVACY_COMPUTING_ANALYZER_H
