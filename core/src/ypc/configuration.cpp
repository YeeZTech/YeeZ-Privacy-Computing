#include "ypc/configuration.h"
#include "ypc/filesystem.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <stdlib.h>

namespace ypc {

std::string configuration::create_logdir_if_not_exist() {
  boost::filesystem::path h(ypc::home_directory());
  h = h / boost::filesystem::path(".yeez.log/");
  if (!boost::filesystem::exists(h)) {
    bool t = boost::filesystem::create_directory(h);
    if (!t) {
      std::stringstream ss;
      ss << "Cannot create log directory " << h.generic_string();
      throw std::runtime_error(ss.str());
    }
  } else if (!boost::filesystem::is_directory(h)) {
    std::stringstream ss;
    ss << h.generic_string() << " is already exist, yet it's not a directory. ";
    throw std::runtime_error(ss.str());
  }
  return h.generic_string();
}

std::string configuration::find_db_config_file(const std::string &filename) {
  boost::filesystem::path full_path(boost::filesystem::current_path());
  // Search filename in the following directories
  // 1. search current dir
  boost::filesystem::path p1 = full_path / boost::filesystem::path(filename);
  if (boost::filesystem::exists(p1)) {
    return p1.generic_string();
  }

  // 2. search home dir
  boost::filesystem::path home(ypc::home_directory());
  boost::filesystem::path p2 = home / boost::filesystem::path(filename);
  if (boost::filesystem::exists(p2)) {
    return p2.generic_string();
  }
  // 3. search /etc/yeez/mysql_db.conf
  boost::filesystem::path f("/etc/yeez/");
  f = f / boost::filesystem::path(filename);
  if (boost::filesystem::exists(f)) {
    return f.generic_string();
  }
  throw std::runtime_error("not found any mysql db configure");
}

db_info_t configuration::read_db_config_file(const std::string &filename) {
  std::string fp = find_db_config_file(filename);
  namespace bp = boost::program_options;

  bp::options_description conf("DB Config");
  // clang-format off
  conf.add_options()
    ("mysql.url", bp::value<std::string>(), "MySQL connection url")
    ("mysql.usr-name", bp::value<std::string>(), "MySQL user name")
    ("mysql.usr-password", bp::value<std::string>(), "MySQL user password")
    ("mysql.database", bp::value<std::string>(), "the database to use");
  // clang-format on

  bp::variables_map vm;
  std::ifstream ifs{fp};
  if (!ifs) {
    std::stringstream ss;
    ss << "cannot open file " << fp;
    throw std::runtime_error(ss.str());
  }
  bp::store(bp::parse_config_file(ifs, conf, true), vm);

  if (!vm.count("mysql.url")) {
    throw std::runtime_error("no mysql.url in config file");
  }
  if (!vm.count("mysql.usr-name")) {
    throw std::runtime_error("no mysql.usr-name in config file");
  }
  if (!vm.count("mysql.usr-password")) {
    throw std::runtime_error("no mysql.usr-password in config file");
  }
  if (!vm.count("mysql.database")) {
    throw std::runtime_error("no mysql.database in config file");
  }
  db_info_t info;
  info.set<db_url>(vm["mysql.url"].as<std::string>());
  info.set<db_usr>(vm["mysql.usr-name"].as<std::string>());
  info.set<db_pass>(vm["mysql.usr-password"].as<std::string>());
  info.set<db_dbname>(vm["mysql.database"].as<std::string>());
  return info;
}
net_info_t
configuration::read_net_info_from_file(const std::string &conf_file) {
  std::string fp = find_db_config_file(conf_file);
  namespace bp = boost::program_options;

  bp::options_description conf("Network Config");
  // clang-format off
  conf.add_options()
    ("net.control-port", bp::value<uint16_t>(), "TCP port for control");
  // clang-format on

  bp::variables_map vm;
  std::ifstream ifs{fp};
  if (!ifs) {
    std::stringstream ss;
    ss << "cannot open file " << fp;
    throw std::runtime_error(ss.str());
  }

  bp::store(bp::parse_config_file(ifs, conf, true), vm);

  if (!vm.count("net.control-port")) {
    throw std::runtime_error("no net.control-port in config file");
  }
  net_info_t info;
  info.set<ctrl_net_port>(vm["net.control-port"].as<uint16_t>());
  return info;
}
} // namespace ypc

