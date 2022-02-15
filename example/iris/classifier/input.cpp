#include "corecommon/package.h"
#include "model.h"
#include "ypc/byte.h"
#include <boost/program_options.hpp>
#include <iostream>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Generate Iris classify data");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("sepal-len", bp::value<double>(), "sepal len")
    ("sepal-wid", bp::value<double>(), "sepal width")
    ("petal-len", bp::value<double>(), "petal len")
    ("petal-wid", bp::value<double>(), "petal width");

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

  iris_data_t md1;
  md1.set<sepal_len, sepal_wid, petal_len, petal_wid>(
      vm["sepal-len"].as<double>(), vm["sepal-wid"].as<double>(),
      vm["petal-len"].as<double>(), vm["petal-wid"].as<double>());

  auto ret = ypc::make_bytes<ypc::bytes>::for_package(md1);
  std::cout << ret << std::endl;

  return 0;
}
