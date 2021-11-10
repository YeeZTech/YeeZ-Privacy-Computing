#include "ypc/command_executor.h"
#include "ypc/configuration.h"
#include "ypc/filesystem.h"
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

TEST(test_configuration, create_logdir_if_not_exist) {
  boost::filesystem::path h(ypc::home_directory());
  h = h / boost::filesystem::path(".yeez.log/");
  EXPECT_EQ(h.generic_string(),
            ypc::configuration::instance().create_logdir_if_not_exist());
}

TEST(test_configuration, find_db_config_file) {
  auto &ins = ypc::configuration::instance();
  std::string filename("xxx");
  EXPECT_THROW(ins.find_db_config_file(filename), std::runtime_error);

  filename = "xx";
  ypc::command_executor::execute_command(
      "sudo -S rm -rf /etc/yeez/xx ~/xx ./xx");
  ypc::command_executor::execute_command(
      "sudo -S mkdir -p /etc/yeez && sudo -S touch /etc/yeez/xx");
  auto ret = ins.find_db_config_file(filename);
  EXPECT_EQ(ret, "/etc/yeez/xx");

  ypc::command_executor::execute_command("touch ~/xx");
  ret = ins.find_db_config_file(filename);
  boost::filesystem::path home(ypc::home_directory());
  boost::filesystem::path p2 = home / boost::filesystem::path(filename);
  EXPECT_EQ(ret, p2.generic_string());

  ypc::command_executor::execute_command("touch ./xx");
  ret = ins.find_db_config_file(filename);
  boost::filesystem::path cur_path(boost::filesystem::current_path());
  boost::filesystem::path p1 = cur_path / boost::filesystem::path(filename);
  EXPECT_EQ(ret, p1.generic_string());
}

TEST(test_configuration, read_db_config_file) {
  auto &ins = ypc::configuration::instance();
  std::string filename("gtest_mysql.conf");
  ypc::command_executor::execute_command("rm -rf " + filename);
  EXPECT_THROW(ins.read_db_config_file(filename), std::runtime_error);
  ypc::command_executor::execute_command("touch " + filename);
  ypc::command_executor::execute_command("echo \"[mysql]\" >> " + filename);
  EXPECT_THROW(ins.read_db_config_file(filename), std::runtime_error);
  ypc::command_executor::execute_command(
      "echo \"url = tcp://127.0.0.1:3306\" >> " + filename);
  EXPECT_THROW(ins.read_db_config_file(filename), std::runtime_error);
  ypc::command_executor::execute_command("echo \"usr-name = xx\" >> " +
                                         filename);
  EXPECT_THROW(ins.read_db_config_file(filename), std::runtime_error);
  ypc::command_executor::execute_command("echo \"usr-password = yy\" >> " +
                                         filename);
  EXPECT_THROW(ins.read_db_config_file(filename), std::runtime_error);
  ypc::command_executor::execute_command("echo \"database = zz\" >> " +
                                         filename);
  auto ret = ins.read_db_config_file(filename);
  EXPECT_EQ(ret.get<db_url>(), "tcp://127.0.0.1:3306");
  EXPECT_EQ(ret.get<db_usr>(), "xx");
  EXPECT_EQ(ret.get<db_pass>(), "yy");
  EXPECT_EQ(ret.get<db_dbname>(), "zz");
}

TEST(test_configuration, read_net_info_from_file) {
  auto &ins = ypc::configuration::instance();
  std::string filename("gtest_net.conf");
  ypc::command_executor::execute_command("rm -rf " + filename);
  EXPECT_THROW(ins.read_db_config_file(filename), std::runtime_error);
  ypc::command_executor::execute_command("touch " + filename);
  ypc::command_executor::execute_command("echo \"[net]\" >> " + filename);
  EXPECT_THROW(ins.read_db_config_file(filename), std::runtime_error);
  ypc::command_executor::execute_command("echo \"control-port = 9876\" >> " +
                                         filename);
  auto ret = ins.read_net_info_from_file(filename);
  EXPECT_EQ(ret.get<ctrl_net_port>(), 9876);
}
