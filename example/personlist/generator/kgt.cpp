#include "ypc/core/byte.h"
#include "ypc/core/kgt_json.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <glog/logging.h>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Key generator tree options");

  // clang-format off
  all.add_options()
    ("help", "")
    ("input", bp::value<std::string>(), "input kgt JSON file");
  // clang-format on

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help") != 0u) {
    std::cout << all << std::endl;
    exit(-1);
  }
  if (vm.count("input") == 0u) {
    std::cerr << "parameter `input` should be specified!" << std::endl;
    exit(-1);
  }
  return vm;
}

std::string read_file(const std::string &filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    std::cerr << "open file failed: " << filename << std::endl;
    exit(-1);
  }
  ifs.seekg(0, std::ios::end);
  size_t size = ifs.tellg();
  std::string buf(size, ' ');
  ifs.seekg(0);
  ifs.read(&buf[0], size);
  return buf;
}
void write_file(const std::string &filename, const std::string &content) {
  std::ofstream ofs;
  ofs.open(filename);
  if (!ofs.is_open()) {
    std::cerr << "open file failed: " << filename << std::endl;
    exit(-1);
  }
  ofs << content;
  ofs.close();
}

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  // read file input
  auto input_file = vm["input"].as<std::string>();
  auto content = read_file(input_file);

  // kgt-sum
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> skey_kgt(content);
  skey_kgt.calculate_kgt_sum();
  const auto &sum = skey_kgt.sum();
  ypc::bytes skey(sum.data, sizeof(sum));
  std::stringstream ss;
  ss << skey;
  boost::property_tree::ptree pt;
  pt.put("private-key", ss.str());
  ss.str("");
  ss.clear();
  ypc::bytes pkey(ypc::crypto::secp256k1::get_public_key_size());
  ypc::crypto::secp256k1::generate_pkey_from_skey(skey.data(), skey.size(),
                                                  pkey.data(), pkey.size());
  ss << pkey;
  pt.put("public-key", ss.str());
  boost::property_tree::json_parser::write_json("kgt-sum.json", pt);

  // kgt-pkey
  auto pkey_node = skey_kgt.gen_pkey_kgt_from_skey_kgt(skey_kgt.root());
  ypc::kgt_json<ypc::crypto::secp256k1_pkey_group> pkey_kgt(pkey_node);
  ss.str("");
  ss.clear();
  ss << pkey_kgt.to_bytes();
  pt = boost::property_tree::ptree();
  pt.put("flat-kgt", ss.str());
  boost::property_tree::json_parser::write_json("kgt-pkey.json", pt);
  return 0;
}
