#pragma once
#include "ypc/db.h"
#include <ff/sql.h>
#include <memory>

define_column(data_id, key, std::string, "DataHash");
define_column(s_name, column, std::string, "DataName");
define_column(ts, column, uint64_t, "TimeStamp");
define_column(path, column, std::string, "FilePath");
define_column(extra_path, column, std::string, "ExtraPath");
define_column(s_index, column, uint64_t, "SliceIndex");

define_column(data_hash, index, std::string, "PDataHash");
define_column(enclave_hash, index, std::string, "EnclaveHash");
define_column(s_status, column, uint64_t, "Status");

namespace toolkit {
namespace ypcd {

struct privacy_data_table_desc {
  constexpr static const char *table_name = "PrivacyDataMeta";
};
struct privacy_analytic_table_desc {
  constexpr static const char *table_name = "PrivacyAnalyticMeta";
};
struct privacy_history_table_desc {
  constexpr static const char *table_name = "AnalyticHistory";
};

typedef ::ff::sql::table<::ff::sql::mysql<::ff::sql::cppconn>,
                         privacy_data_table_desc, data_id, s_name, ts,
                         extra_path>
    privacy_data_table;
typedef typename privacy_data_table::row_type privacy_data_item_t;

typedef ::ff::sql::table<::ff::sql::mysql<::ff::sql::cppconn>,
                         privacy_analytic_table_desc, enclave_hash, data_hash,
                         s_name, path, ts>
    analytic_program_table;

typedef ::ff::sql::table<::ff::sql::mysql<::ff::sql::cppconn>,
                         privacy_history_table_desc, data_id, data_hash,
                         enclave_hash, ts, s_status>
    analytic_history_table;

class ypcd_db : public ypc::db_base {
public:
  ypcd_db(const std::string &url, const std::string &usrname,
          const std::string &passwd, const std::string &dbname);
  virtual void create_tables();
  virtual void clear_tables();
};
} // namespace ypcd
} // namespace toolkit

