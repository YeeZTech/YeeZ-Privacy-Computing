#include "ypc/db.h"

namespace ypc {

db_base::db_base(const std::string &url, const std::string &usrname,
                 const std::string &passwd, const std::string &dbname) {
  init_db(url, usrname, passwd, dbname);
}

void db_base::init_db(const std::string &url, const std::string &usrname,
                      const std::string &passwd, const std::string &dbname) {
  m_db_engine = std::make_shared<::ff::sql::mysql<::ff::sql::cppconn>>(
      url, usrname, passwd, dbname);
}

::ff::sql::mysql<::ff::sql::cppconn> *db_base::db_engine_ptr() {
  return m_db_engine.get();
}
} // namespace ypc
