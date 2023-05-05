#include "iodef.h"
#include "ypc/core/byte.h"
#include "ypc/core/filesystem.h"
#include "ypc/corecommon/package.h"
#include <boost/property_tree/json_parser.hpp>

typedef ::ff::net::ntpackage<0x3b549098, shu_pkey, dian_pkey> pkeys_pkg_t;

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Parameter Generator options");

  // clang-format off
all.add_options()
  ("help", "help message")
  ("dian_pkey", bp::value<std::string>(), "dian public key")
  ("shu_pkey", bp::value<std::string>(), "request shu public key")
  ("output", bp::value<std::string>(), "output file");
  // clang-format on

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

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
  } catch (...) {
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  auto dpkey =
      ypc::hex_bytes(vm["dian_pkey"].as<std::string>()).as<ypc::bytes>();
  auto spkey =
      ypc::hex_bytes(vm["shu_pkey"].as<std::string>()).as<ypc::bytes>();
  pkeys_pkg_t pkg;
  pkg.set<::dian_pkey, ::shu_pkey>(dpkey, spkey);
  auto pkg_bytes = ypc::make_bytes<ypc::bytes>::for_package(pkg);

  if (vm.count("output") != 0u) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    std::stringstream ss;
    ss << pkg_bytes;
    boost::property_tree::ptree pt;
    pt.put("package", ss.str());
    boost::property_tree::json_parser::write_json(output_path, pt);
  } else {
    std::cout << pkg_bytes << std::endl;
  }
  return 0;
}
