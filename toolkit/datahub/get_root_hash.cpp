#include "ypc/core/oramblockfile.h"
#include "ypc/core/version.h"

#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Read root hash options");
  bp::options_description general("General Options");
  bp::options_description seal_data_opts("Seal Data Options");

  // clang-format off
  seal_data_opts.add_options()
    ("data-url", bp::value<std::string>(), "Data URL");

  general.add_options()
    ("help", "help message")
    ("version", "show version");

  // clang-format on

  all.add(general).add(seal_data_opts);

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


int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;

  try {
    vm = parse_command_line(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }
  if (vm.count("data-url") == 0u) {
    std::cerr << "data not specified!" << std::endl;
    return -1;
  }

  std::string data_file = vm["data-url"].as<std::string>();
  std::ofstream ofs;
  ofs.open(data_file);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << data_file << "\n";
    return -1;
  }
  ofs.close();

  std::string line;
  



  return 0;
}