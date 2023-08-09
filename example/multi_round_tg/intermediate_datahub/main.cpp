#include "ypc/core/ntobject_file.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/version.h"
#include <boost/program_options.hpp>
#include <fstream>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Intermediate Data Hub Options");
  bp::options_description general("General Options");
  bp::options_description seal_data_opts("Seal Data Options");

  // clang-format off
  seal_data_opts.add_options()
    ("encrypted-data-hex", bp::value<std::string>(), "Encrypted Data Hex")
    ("sealed-data-url", bp::value<std::string>(), "Sealed Data URL");

  general.add_options()
    ("help", "Help Message")
    ("version", "Show Version");
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

  if (vm.count("encrypted-data-hex") == 0u) {
    std::cerr << "data not specified!" << std::endl;
    return -1;
  }
  if (vm.count("sealed-data-url") == 0u) {
    std::cerr << "sealed data url not specified" << std::endl;
    return -1;
  }

  std::string data_file = vm["encrypted-data-hex"].as<std::string>();
  std::string sealed_data_file = vm["sealed-data-url"].as<std::string>();

  // read encrypted data hex
  ypc::bytes buf = ypc::hex_bytes(vm["encrypted-data-hex"].as<std::string>())
                       .as<ypc::bytes>();

  ypc::simple_sealed_file sf(sealed_data_file, false);
  // TODO consider the size of data file is very large
  sf.write_item(buf);

  std::cout << "done sealing" << std::endl;
  return 0;
}
