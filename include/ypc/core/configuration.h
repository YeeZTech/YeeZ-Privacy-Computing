
#pragma once
#include "ypc/core/singleton.h"
#include <ff/util/ntobject.h>
#include <string>

define_nt(db_url, std::string);
define_nt(db_usr, std::string);
define_nt(db_pass, std::string);
define_nt(db_dbname, std::string);
define_nt(ctrl_net_port, uint16_t);

namespace ypc {

using db_info_t = ::ff::util::ntobject<db_url, db_usr, db_pass, db_dbname>;
using net_info_t = ::ff::util::ntobject<ctrl_net_port>;

class configuration : public singleton<configuration> {
public:
  configuration() = default;
  configuration(const configuration &cf) = delete;
  configuration &operator=(const configuration &cf) = delete;
  configuration &operator=(configuration &&cf) = delete;
  configuration(configuration &&cf) = delete;
  ~configuration() = default;

public:
  static std::string create_logdir_if_not_exist();
  static std::string find_db_config_file(const std::string &filename);

  static db_info_t read_db_config_file(const std::string &filename);
  static net_info_t read_net_info_from_file(const std::string &conf_file);
};

} // namespace ypc
