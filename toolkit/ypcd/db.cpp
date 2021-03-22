#include "db.h"

namespace toolkit {
namespace ypcd {

ypcd_db::ypcd_db(const std::string &url, const std::string &usrname,
                 const std::string &passwd, const std::string &dbname)
    : db_base(url, usrname, passwd, dbname) {}

void ypcd_db::create_tables() {
  privacy_data_table::create_table(m_db_engine.get());
  analytic_program_table::create_table(m_db_engine.get());
  analytic_history_table::create_table(m_db_engine.get());
}
void ypcd_db::clear_tables() {
  privacy_data_table::drop_table(m_db_engine.get());
  analytic_program_table::drop_table(m_db_engine.get());
  analytic_history_table::drop_table(m_db_engine.get());
}
} // namespace ypcd
} // namespace toolkit
