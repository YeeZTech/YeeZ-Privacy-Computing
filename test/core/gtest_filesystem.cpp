#include "ypc/command_executor.h"
#include "ypc/filesystem.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

TEST(test_filesystem, home_directory) {
  std::string output;
  ypc::command_executor::execute_command("cd ~ && pwd", output);
  boost::replace_all(output, "\n", "");
  EXPECT_EQ(output, ypc::home_directory());
}

TEST(test_filesystem, current_directory) {
  std::string output;
  ypc::command_executor::execute_command("pwd", output);
  boost::replace_all(output, "\n", "");
  EXPECT_EQ(output, ypc::current_directory());
}

TEST(test_filesystem, is_portable_name) {
  EXPECT_TRUE(ypc::is_portable_name("_123"));
  EXPECT_FALSE(ypc::is_portable_name(";123"));
  EXPECT_FALSE(ypc::is_portable_name("_123~"));
  EXPECT_FALSE(ypc::is_portable_name("_12/3"));
  EXPECT_FALSE(ypc::is_portable_name("[123"));
}

TEST(test_filesystem, complete_path) {
  std::string output;
  ypc::command_executor::execute_command("pwd", output);
  boost::replace_all(output, "\n", "");
  auto cp = ypc::complete_path("./xx");
  EXPECT_EQ(cp, output + "/./xx");
}

TEST(test_filesystem, is_dir_exists) {
  boost::filesystem::path h(ypc::current_directory());
  h = h / boost::filesystem::path("/xx");
  ypc::command_executor::execute_command("rm -rf xx");
  EXPECT_FALSE(ypc::is_dir_exists(h.generic_string()));
  ypc::command_executor::execute_command("mkdir xx");
  EXPECT_TRUE(ypc::is_dir_exists(h.generic_string()));
  ypc::command_executor::execute_command("rm -rf xx");
  EXPECT_FALSE(ypc::is_dir_exists(h.generic_string()));

  ypc::command_executor::execute_command("touch xx");
  EXPECT_FALSE(ypc::is_dir_exists(h.generic_string()));
}

TEST(test_filesystem, is_file_exists) {
  boost::filesystem::path h(ypc::current_directory());
  h = h / boost::filesystem::path("/xx");
  ypc::command_executor::execute_command("rm -rf xx");
  EXPECT_FALSE(ypc::is_file_exists(h.generic_string()));
  ypc::command_executor::execute_command("touch xx");
  EXPECT_TRUE(ypc::is_file_exists(h.generic_string()));
  ypc::command_executor::execute_command("rm -rf xx");
  EXPECT_FALSE(ypc::is_file_exists(h.generic_string()));

  ypc::command_executor::execute_command("mkdir xx");
  EXPECT_FALSE(ypc::is_file_exists(h.generic_string()));
}
