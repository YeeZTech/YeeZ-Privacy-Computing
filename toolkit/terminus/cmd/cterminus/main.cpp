#include "cmd_line.h"

int main(int argc, char *argv[]) {
  auto tp = parse_command_line(argc, argv);
  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  return std::get<1>(tp)(crypto.get());
}
