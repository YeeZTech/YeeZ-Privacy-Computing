
#pragma once
#include "singleton.h"
#include <ff/util/ntobject.h>
#include <string>

define_nt(db_url, std::string);
define_nt(db_usr, std::string);
define_nt(db_pass, std::string);
define_nt(db_dbname, std::string);
define_nt(ctrl_net_port, uint16_t);

namespace ypc {

typedef ::ff::util::ntobject<db_url, db_usr, db_pass, db_dbname> db_info_t;
typedef ::ff::util::ntobject<ctrl_net_port> net_info_t;

class configuration : public singleton<configuration> {
public:
  configuration() = default;
  configuration(const configuration &cf) = delete;
  configuration &operator=(const configuration &cf) = delete;
  configuration(configuration &&cf) = delete;
  ~configuration() = default;

public:
  std::string create_logdir_if_not_exist();
  std::string find_db_config_file(const std::string &filename);

  db_info_t read_db_config_file(const std::string &filename);
  net_info_t read_net_info_from_file(const std::string &conf_file);

  inline std::string contract_upload_method_id() const { return "0x58f4f6a7"; }
  inline std::string contract_buy_method_id() const { return "0xa6f2ae3a"; }
  inline std::string contract_meta_method_id() const { return "0x074c8841"; }
  inline std::string contract_sell_method_id() const { return "0xeb48e53c"; }
};

} // namespace ypc
