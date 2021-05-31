#include "params/param_from_db.h"

using namespace toolkit::analyzer;

param_from_db::param_from_db(const std::string &url, const std::string &usrname,
                             const std::string &passwd,
                             const std::string &dbname,
                             const ::ypc::bytes &request_hash)
    : m_request_hash(request_hash) {
  m_db = std::make_unique<request_db>(url, usrname, passwd, dbname);
}

uint32_t param_from_db::read_from_source() {
  auto *ptr = m_db->db_engine_ptr();
  auto info =
      request_data_table::select<::encrypted_skey, ::encrypted_input,
                                 ::provider_pkey, ::enclave_hash,
                                 ::analyzer_pkey, ::forward_sig>(ptr)
          .where(request_hash::eq(m_request_hash.as<::ypc::hex_bytes>()))
          .eval();
  if (info.empty()) {
    return ::ypc::param_from_db_read_from_source_failed;
  }
  assert(info.size() == 1);
  m_eskey = info[0].get<::encrypted_skey>().as<::ypc::bytes>();
  m_input = info[0].get<::encrypted_input>().as<::ypc::bytes>();
  m_epkey = info[0].get<::provider_pkey>().as<::ypc::bytes>();
  m_ehash = info[0].get<::enclave_hash>().as<::ypc::bytes>();
  m_vpkey = info[0].get<::analyzer_pkey>().as<::ypc::bytes>();
  m_sig = info[0].get<::forward_sig>().as<::ypc::bytes>();
  return ::ypc::success;
}
