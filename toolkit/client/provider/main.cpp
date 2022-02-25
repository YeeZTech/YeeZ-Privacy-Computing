#include "stbox/eth/eth_hash.h"
#include "ypc/byte.h"
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cmath>
#include <ff/util/ntobject.h>
#include <fstream>
#include <iostream>
#include <keymgr/default/keymgr_sgx_module.h>
#include <sstream>

#define ENCLAVE_KEYMGR_PATH "../lib/keymgr.signed.so"
#define VARCHAR_MAX_LENGTH 255
#define TEXT_MAX_LENGTH 4096
#define PKEY_SIZE (64 * 2)
#define SIGNATURE_SIZE (65 * 2)

define_nt(data_url, std::string);
define_nt(sealed_data_url, std::string);
define_nt(sealer_enclave, std::string);
define_nt(data_id, std::string);
define_nt(item_num, uint64_t);

typedef ::ff::util::ntobject<data_url, sealed_data_url, sealer_enclave, data_id,
                             item_num>
    sealed_info_t;

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all(
      "YeeZ Privacy Data Explorer and Prepare for Analyzer");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("sealed-path", bp::value<std::string>(), "sealed.output file path")
    ("program-proxy", bp::value<std::string>(), "ProgramStore contract address")
    ("output", bp::value<std::string>(), "output file name");

  // clang-format on
  bp::variables_map vm;
  boost::program_options::store(bp::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }

  return vm;
}

bool all_number(const std::string &s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](char ch) {
                         return !std::isdigit(ch);
                       }) == s.end();
}

void long_text_exception(const std::string &text, size_t length) {
  if (text.size() > length) {
    throw std::runtime_error("Text is too long");
  }
}

void read_from_stdin(const std::string &tips, size_t max_len,
                     std::string &content) {
  std::cout << tips;
  // std::cin >> content;
  std::getline(std::cin, content);
  long_text_exception(content, max_len);
}

std::string write_pt_to_json_str(const boost::property_tree::ptree &pt) {
  std::stringstream ss;
  try {
    boost::property_tree::json_parser::write_json(ss, pt, false);
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format("Write pt to string failed! Error: %1%") % e.what()));
  }
  return ss.str();
}

std::string write_env_info_to_json(const std::string &os,
                                   const std::string &kernel,
                                   const std::string &sgx,
                                   const std::string &processor,
                                   const std::string &compiler) {
  boost::property_tree::ptree pt;
  pt.put("os", os);
  pt.put("kernel", kernel);
  pt.put("sgx", sgx);
  pt.put("processor", processor);
  pt.put("compiler", compiler);
  return write_pt_to_json_str(pt);
}

void read_env_info_from_stdin(std::string &env_info) {
  std::string os, kernel, sgx, processor, compiler;
  std::cout << "Please input environment infomation \n";
  // input os info
  read_from_stdin("\tPlease input os info: ", VARCHAR_MAX_LENGTH, os);
  // input kernel info
  read_from_stdin("\tPlease input kernel info: ", VARCHAR_MAX_LENGTH, kernel);
  // input sgx info
  read_from_stdin("\tPlease input sgx info: ", VARCHAR_MAX_LENGTH, sgx);
  // input processor info
  read_from_stdin("\tPlease input processor info: ", VARCHAR_MAX_LENGTH,
                  processor);
  // input compiler info
  read_from_stdin("\tPlease input compiler info: ", VARCHAR_MAX_LENGTH,
                  compiler);
  env_info = write_env_info_to_json(os, kernel, sgx, processor, compiler);
}

void read_params_from_stdin(std::string &name, std::string &desc,
                            std::string &sample, std::string &env_info,
                            uint64_t &price,
                            std::string &pkey, std::string &pkey_sig) {
  // input name
  read_from_stdin("Please input data name: ", VARCHAR_MAX_LENGTH, name);

  // input desc
  read_from_stdin("Please input data description: ", TEXT_MAX_LENGTH, desc);

  // input sample
  read_from_stdin("Please input data sample url: ", TEXT_MAX_LENGTH, sample);

  // input env_info
  read_env_info_from_stdin(env_info);

  // input price
  std::string str_price;
  read_from_stdin("Please input data price: ", log10(0xffffffffffffffff) - 1,
                  str_price);
  if (!all_number(str_price)) {
    throw std::runtime_error("Invalid price");
  }
  price = std::stoll(str_price);

  std::cout << "Please input public key: ";
  std::cin >> pkey;
  if (PKEY_SIZE != pkey.size()) {
    throw std::runtime_error("Invalid public key length");
  }

  std::cout << "Please input signature of public key: ";
  std::cin >> pkey_sig;
  if (SIGNATURE_SIZE != pkey_sig.size()) {
    throw std::runtime_error("Invalid signature length");
  }
}

std::string add_prefix_hex_if_not_exist(const std::string &hex_str) {
  if (hex_str.substr(0, 2) == "0x") {
    return hex_str;
  }
  return "0x" + hex_str;
}

boost::property_tree::ptree generate_description(const std::string &name,
                                                 const std::string &desc,
                                                 uint64_t item_num) {
  boost::property_tree::ptree pt;
  // format
  std::string tmp("yeez.tech#" + name + "#csv");
  ypc::bytes b(tmp);
  auto b_hash = stbox::eth::keccak256_hash(b);
  std::stringstream ss;
  ss << b_hash;
  std::string format(tmp + '#' + ss.str().substr(0, 8));
  pt.add("format", format);
  // field
  pt.add("field", "");
  // tag
  boost::property_tree::ptree child;
  boost::property_tree::ptree empty;
  child.push_back(std::make_pair("", empty));
  pt.add_child("tag", child);
  // item_num
  pt.add("item_num", item_num);
  // detail
  pt.add("detail", desc);
  return pt;
}

void write_params_to_json(const std::string &filename, const std::string &hash,
                          uint64_t item_num, const std::string &name,
                          const std::string &desc, const std::string &sample,
                          const std::string &sample_hex,
                          const std::string &env_info, uint64_t &price,
                          const std::string &program_proxy,
                          const std::string &pkey,
                          const std::string &pkey_sig) {
  try {
    boost::property_tree::ptree pt;
    pt.put("data_hash", hash);
    pt.put("name", name);
    boost::property_tree::ptree child =
        generate_description(name, desc, item_num);
    pt.add_child("desc", child);
    pt.put("sample", sample);
    pt.put("sample_hex", sample_hex);
    pt.put("env_info", env_info);
    pt.put("price", price);
    pt.put("program_proxy", program_proxy);
    pt.put("pkey", pkey);
    pt.put("pkey_sig", pkey_sig);
    boost::property_tree::json_parser::write_json(filename, pt);
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format("Write key pair to file failed! Error: %1%") % e.what()));
  }
}

sealed_info_t read_sealed_output_file(const std::string &filename) {
  std::string fp = filename;
  namespace bp = boost::program_options;

  bp::options_description conf("Sealed Output File");
  // clang-format off
  conf.add_options()
    ("data_url", bp::value<std::string>(), "data url")
    ("sealed_data_url", bp::value<std::string>(), "sealed data url")
    ("sealer_enclave", bp::value<std::string>(), "sealer enclave")
    ("data_id", bp::value<std::string>(), "data hash")
    ("item_num", bp::value<uint64_t>(), "item number");
  // clang-format on

  bp::variables_map vm;
  std::ifstream ifs{fp};
  if (!ifs) {
    std::stringstream ss;
    ss << "cannot open file " << fp;
    throw std::runtime_error(ss.str());
  }
  bp::store(bp::parse_config_file(ifs, conf, true), vm);

  if (!vm.count("data_url")) {
    throw std::runtime_error("no data_url in sealed.output file");
  }
  if (!vm.count("sealed_data_url")) {
    throw std::runtime_error("no sealed_data_url in sealed.output file");
  }
  if (!vm.count("sealer_enclave")) {
    throw std::runtime_error("no sealer_enclave in sealed.output file");
  }
  if (!vm.count("data_id")) {
    throw std::runtime_error("no data_id in sealed.output file");
  }
  if (!vm.count("item_num")) {
    throw std::runtime_error("no item_num in sealed.output file");
  }
  sealed_info_t info;
  info.set<data_url>(vm["data_url"].as<std::string>());
  info.set<sealed_data_url>(vm["sealed_data_url"].as<std::string>());
  info.set<sealer_enclave>(vm["sealer_enclave"].as<std::string>());
  info.set<data_id>(vm["data_id"].as<std::string>());
  info.set<item_num>(vm["item_num"].as<uint64_t>());
  return info;
}

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cout << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  if (!vm.count("sealed-path")) {
    std::cout << "No sealed.output file path is provided!" << std::endl;
    return -1;
  }
  std::string sealed_path = vm["sealed-path"].as<std::string>();
  auto info = read_sealed_output_file(sealed_path);

  if (!vm.count("program-proxy")) {
    std::cout << "No program proxy is provided!" << std::endl;
    return -1;
  }
  std::string program_proxy = vm["program-proxy"].as<std::string>();

  if (!vm.count("output")) {
    std::cout << "No output file is provided!" << std::endl;
    return -1;
  }
  std::string output = vm["output"].as<std::string>();

  std::string name, desc, sample, env_info;
  uint64_t price;
  std::string pkey, pkey_sig;
  read_params_from_stdin(name, desc, sample, env_info, price, pkey, pkey_sig);

  auto sample_hex_bytes = ypc::bytes(sample.c_str()).as<ypc::hex_bytes>();
  std::string sample_hex(
      (const char *)sample_hex_bytes.data(),
      sample_hex_bytes.size()); // ypc::to_hex(ypc::string_to_byte(sample));

  write_params_to_json(
      output, add_prefix_hex_if_not_exist(info.get<::data_id>()),
      info.get<::item_num>(), name, desc, sample,
      add_prefix_hex_if_not_exist(sample_hex), env_info, price,
      add_prefix_hex_if_not_exist(program_proxy),
      add_prefix_hex_if_not_exist(pkey), add_prefix_hex_if_not_exist(pkey_sig));
  return 0;
}
