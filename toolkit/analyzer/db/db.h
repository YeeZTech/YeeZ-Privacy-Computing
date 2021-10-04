#pragma once
#include "ypc/byte.h"
#include "ypc/db.h"
#include <ff/sql.h>
#include <memory>

define_column(request_hash, index, ypc::hex_bytes, "request_hash");
define_column(encrypted_skey, column, ypc::hex_bytes, "encrypted_skey");
define_column(encrypted_input, column, ypc::hex_bytes, "encrypted_input");
define_column(provider_pkey, column, ypc::hex_bytes, "provider_pkey");
define_column(analyzer_pkey, column, ypc::hex_bytes, "analyzer_pkey");
define_column(enclave_hash, column, ypc::hex_bytes, "program_enclave_hash");
define_column(forward_sig, column, ypc::hex_bytes, "forward_sig");

define_column(status, column, uint64_t, "status");
define_column(encrypted_result, column, ypc::hex_bytes, "encrypted_result");
define_column(result_signature, column, ypc::hex_bytes, "result_signature");
define_column(cost_signature, column, ypc::hex_bytes, "cost_signature");
define_column(data_hash, column, ypc::hex_bytes, "data_hash");

namespace ff {
namespace sql {
template <class STMT> struct mysql_bind_setter<STMT, ypc::hex_bytes> {
  static void bind(STMT stmt, int index, const ypc::hex_bytes &value) {
    stmt->setString(index,
                    std::string((const char *)value.data(), value.size()));
  }
};

template <> struct mysql_rs_getter<ypc::hex_bytes> {
  template <typename RST>
  static ypc::hex_bytes get(RST r, const std::string &name) {
    std::string str = r->getString(name);
    return ypc::hex_bytes(str.c_str(), str.size());
  }
};
} // namespace sql
} // namespace ff
/*
define_column(request_hash, index, std::string, "request_hash");
define_column(encrypted_skey, column, std::string, "encrypted_skey");
define_column(encrypted_input, column, std::string, "encrypted_input");
define_column(provider_pkey, column, std::string, "provider_pkey");
define_column(analyzer_pkey, column, std::string, "analyzer_pkey");
define_column(enclave_hash, column, std::string, "program_enclave_hash");
define_column(forward_sig, column, std::string, "forward_sig");

define_column(status, column, uint64_t, "status");
define_column(encrypted_result, column, std::string, "encrypted_result");
define_column(result_signature, column, std::string, "result_signature");
define_column(data_hash, column, std::string, "data_hash");
*/

namespace toolkit {
namespace analyzer {

struct request_data_table_desc {
  constexpr static const char *table_name = "RequestData";
};

typedef ::ff::sql::table<::ff::sql::mysql<::ff::sql::cppconn>,
                         request_data_table_desc, request_hash, encrypted_skey,
                         encrypted_input, provider_pkey, analyzer_pkey,
                         enclave_hash, forward_sig, status, encrypted_result,
                         result_signature, cost_signature, data_hash>
    request_data_table;
typedef typename request_data_table::row_type request_data_item_t;

class request_db : public ypc::db_base {
public:
  request_db(const std::string &url, const std::string &usrname,
             const std::string &passwd, const std::string &dbname);
  virtual void create_tables();
  virtual void clear_tables();
};
} // namespace ypcd
} // namespace toolkit

