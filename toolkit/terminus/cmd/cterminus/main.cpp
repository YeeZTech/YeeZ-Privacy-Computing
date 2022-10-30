#include "cmd_line.h"

int main(int argc, char *argv[]) {
  auto tp = parse_command_line(argc, argv);

  boost::program_options::variables_map vm = std::get<0>(tp);
  std::string crypto_type = "stdeth";
  if (vm.count("crypto")) {
    crypto_type = vm["crypto"].as<std::string>();
  }

  std::unique_ptr<ypc::terminus::crypto_pack> crypto;
  if (crypto_type == "stdeth") {
    crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  } else if (crypto_type == "gmssl") {
    crypto = ypc::terminus::sm_compatible();
  } else {
    std::cerr << "invalid crypto type!" << std::endl;
    return -1;
  }

  return std::get<1>(tp)(crypto.get());
}
