#include "ypc/command_executor.h"
#include <boost/process.hpp>
#include <iostream>
#include <string>

namespace ypc {

namespace internal {
int command_executor_helper::execute_command(const std::string &command,
                                             std::string &output) {
  FILE *stream;
  const int max_buffer = 256;
  char buffer[max_buffer];
  std::string cmd = command;
  cmd.append(" 2>&1");

  stream = popen(cmd.c_str(), "r");
  if (stream) {
    while (!feof(stream)) {
      if (fgets(buffer, max_buffer, stream) != NULL) {
        output.append(buffer);
      }
    }
    pclose(stream);
  }
  return 0;
}
} // namespace internal

int command_executor::execute_command(const std::string &cmd) {
  std::string output;
  return execute_command(cmd, output);
}

int command_executor::execute_command(const std::string &cmd,
                                      std::string &output) {
  return internal::command_executor_helper::execute_command(cmd, output);
}
} // namespace ypc
