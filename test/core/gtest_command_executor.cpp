#include "ypc/command_executor.h"
#include <gtest/gtest.h>

TEST(test_command_executor, without_output) {
  std::string cmd = "touch xx && ls | grep xx ";
  auto ret = ypc::command_executor::execute_command(cmd);
  EXPECT_EQ(ret, 0);
  ypc::command_executor::execute_command("rm -rf xx");
}

TEST(test_command_executor, with_output) {
  std::string output;
  std::string cmd = "touch xx && ls | grep xx ";
  auto ret = ypc::command_executor::execute_command(cmd, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output, "xx\n");
  ypc::command_executor::execute_command("rm -rf xx", output);
}

TEST(test_command_executor, invalid_cmd) {
  std::string output;
  std::string cmd = "xxyyzz";
  auto ret = ypc::command_executor::execute_command(cmd, output);
  ypc::command_executor::execute_command(cmd);
}

