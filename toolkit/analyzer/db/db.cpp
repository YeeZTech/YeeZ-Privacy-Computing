#include "db.h"

namespace toolkit {
namespace analyzer {

request_db::request_db(const std::string &url, const std::string &usrname,
                       const std::string &passwd, const std::string &dbname)
    : db_base(url, usrname, passwd, dbname) {}

void request_db::create_tables() {
  request_data_table::create_table(m_db_engine.get());
}
void request_db::clear_tables() {
  request_data_table::drop_table(m_db_engine.get());
}
} // namespace analyzer
} // namespace toolkit
