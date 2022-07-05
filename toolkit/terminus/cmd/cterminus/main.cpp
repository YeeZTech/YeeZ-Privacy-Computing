#include "cmd_line.h"

int main(int argc, char *argv[]) {
  auto tp = parse_command_line(argc, argv);
  auto crypto = ypc::terminus::sm_compatible();
  return std::get<1>(tp)(crypto.get());
}