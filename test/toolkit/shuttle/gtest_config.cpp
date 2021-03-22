#include "toolkit/shuttle/config.h"
#include "ypc/command_executor.h"
#include "ypc/filesystem.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <gtest/gtest.h>

TEST(test_config, parse_config_file_exception) {
  boost::filesystem::path h(ypc::home_directory());
  h = h / boost::filesystem::path("/xx");
  std::string cmd =
      boost::str(boost::format("rm -rf %1% ./xx") % h.generic_string());
  ypc::command_executor::execute_command(cmd);
  toolkit::shuttle::configure c;
  EXPECT_THROW(c.parse_config_file(h.generic_string()), std::runtime_error);

  cmd = boost::str(boost::format("touch %1%") % h.generic_string());
  ypc::command_executor::execute_command(cmd);
  EXPECT_THROW(c.parse_config_file(h.generic_string()), std::runtime_error);
}

TEST(test_config, parse_config_file) {
  boost::filesystem::path h(ypc::home_directory());
  h = h / boost::filesystem::path("/xx");
  std::string cmd = boost::str(boost::format("rm -rf %1% ./xx && touch %1%") %
                               h.generic_string());
  ypc::command_executor::execute_command(cmd);
  std::vector<std::string> lines{"[general]",
                                 "mode = file",
                                 "[data]",
                                 "sealed_data_url = data.sealed",
                                 "[blockchain]",
                                 "data_type_header = user_type.h",
                                 "data_id = base64str",
                                 "data_parser_lib = libuser_plugin_t.a",
                                 "data_desc = This is Data Description",
                                 "[exec]",
                                 "parser_path = parser.signed.so",
                                 "params = params here"};
  for (auto &line : lines) {
    cmd = boost::str(boost::format("echo %2% >> %3%") % cmd % line %
                     h.generic_string());
    ypc::command_executor::execute_command(cmd);
  }
  toolkit::shuttle::configure c;
  c.parse_config_file("xx");
  EXPECT_EQ(c.data_type_header(), "user_type.h");
  EXPECT_EQ(c.data_parser_lib(), "libuser_plugin_t.a");
  EXPECT_EQ(c.data_id(), "base64str");
  EXPECT_EQ(c.exec_parser_path(), "parser.signed.so");
  EXPECT_EQ(c.exec_params(), "params here");
  EXPECT_EQ(c.sealed_data_url(), "data.sealed");
  EXPECT_EQ(c.data_desc(), "This is Data Description");
}
