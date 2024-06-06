#include "ypc/core/oram_sealed_file.h"
#include "ypc/core/version.h"
#include "ypc/core/oram_sealed_file.h"

#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Read root hash options");
  bp::options_description general("General Options");
  bp::options_description get_root_hash_opts("Seal Data Options");

  // clang-format off
  get_root_hash_opts.add_options()
    ("data-url", bp::value<std::string>(), "Sealed Data URL")
    ("output", bp::value<std::string>(), "root hash file path");

  general.add_options()
    ("help", "help message")
    ("version", "show version");

  // clang-format on

  all.add(general).add(get_root_hash_opts);

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
  if (vm.count("output") == 0u) {
    std::cerr << "output not specified" << std::endl;
    return -1;
  }

  std::string data_file = vm["data-url"].as<std::string>();
  std::string output = vm["output"].as<std::string>();

  auto sosf = std::make_shared<ypc::simple_oram_sealed_file>(data_file);

  sosf->open_for_write();
  
  ypc::memref me_hash;
  bool ret = sosf->read_root_hash(me_hash);
  ypc::bytes root_hash(me_hash.data(), me_hash.size());
  if(!ret) {
    std::cout << "Cannot read root hash" << std::endl;
    return -1;
  }

  std::ofstream ofs;
  ofs.open(output);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }

  ofs << root_hash;
  ofs.close();

  std::cout << root_hash;
  return 0;
}