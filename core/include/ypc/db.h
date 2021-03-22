
#pragma once
#include "ypc/configuration.h"
#include <ff/sql.h>
#include <memory>

namespace ypc {

class db_base {
public:
  db_base(const std::string &url, const std::string &usrname,
          const std::string &passwd, const std::string &dbname);
  virtual void create_tables() = 0;
  virtual void clear_tables() = 0;

  ::ff::sql::mysql<::ff::sql::cppconn> *db_engine_ptr();

protected:
  virtual void init_db(const std::string &url, const std::string &usrname,
                       const std::string &passwd, const std::string &dbname);

protected:
  std::shared_ptr<::ff::sql::mysql<::ff::sql::cppconn>> m_db_engine;
};

template <typename DB>
std::shared_ptr<ypc::db_base> construct_db_ptr(const std::string &filename) {
  db_info_t dbinfo = configuration::instance().read_db_config_file(filename);
  std::shared_ptr<ypc::db_base> base_ptr =
      std::make_shared<DB>(dbinfo.get<db_url>(), dbinfo.get<db_usr>(),
                           dbinfo.get<db_pass>(), dbinfo.get<db_dbname>());
  return base_ptr;
}

} // namespace ypc
