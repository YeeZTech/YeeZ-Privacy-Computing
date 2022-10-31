#pragma once
#include <string>

namespace ypc {

namespace internal {
class command_executor_helper {
public:
  static int execute_command(const std::string &cmd, std::string &output);
};
} // namespace internal

class command_executor {
public:
  static int execute_command(const std::string &cmd);
  static int execute_command(const std::string &cmd, std::string &output);
};

} // namespace ypc
