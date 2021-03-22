#include "params/param_from_db.h"

using namespace toolkit::analyzer;

param_from_db::param_from_db(const std::string &url, const std::string &usrname,
                             const std::string &passwd,
                             const std::string &dbname,
                             const std::string &request_hash)
    : m_request_hash(request_hash) {
  m_db = std::make_unique<request_db>(url, usrname, passwd, dbname);
}

void param_from_db::read_from_source() {
  auto *ptr = m_db->db_engine_ptr();
  auto info = request_data_table::select<::encrypted_skey, ::encrypted_input,
                                         ::provider_pkey, ::enclave_hash,
                                         ::analyzer_pkey, ::forward_sig>(ptr)
                  .where(request_hash::eq(m_request_hash))
                  .eval();
  if (!info.empty()) {
    assert(info.size() == 1);
    m_eskey = ypc::bytes::from_hex(info[0].get<::encrypted_skey>());
    m_input = ypc::bytes::from_hex(info[0].get<::encrypted_input>());
    m_epkey = ypc::bytes::from_hex(info[0].get<::provider_pkey>());
    m_ehash = ypc::bytes::from_hex(info[0].get<::enclave_hash>());
    m_vpkey = ypc::bytes::from_hex(info[0].get<::analyzer_pkey>());
    m_sig = ypc::bytes::from_hex(info[0].get<::forward_sig>());
  }
}
