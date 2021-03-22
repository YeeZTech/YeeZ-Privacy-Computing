#include "config.h"
#include "net.h"
#include "ypc/configuration.h"
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/program_options.hpp>
#include <iostream>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Yeez Privacy Computing Data Shuttle");
  bp::options_description general("General options");
  bp::options_description common("Common options");

  // clang-format off
  general.add_options()
    ("help", "help message")
    //("fp-gen", "generate user file parser plugin")
    //("seal", "seal data")
    //("parse", "run the parser")
    ("onchain", "submit meta to blockchain")
    //("all", "run fp-gen/seal/onchain")
    ("status", "check status of user data");

  common.add_options()
    ("input", bp::value<std::string>(), "input config file");

  // clang-format on

  all.add(general).add(common);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    toolkit::shuttle::configure c;
    std::cout << c.help_message() << std::endl;
    exit(-1);
  }
  return vm;
}

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cout << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  if (vm.count("onchain")) {
    std::string input = vm["input"].as<std::string>();
    toolkit::shuttle::configure conf;
    conf.parse_config_file(input);
    toolkit::shuttle::send_data_meta(
        conf.data_id(), conf.data_desc(), conf.exec_parser_path(),
        conf.exec_params(), conf.sealed_data_url());
  }
  return 0;
}
