#pragma once
#include "common/parser_type.h"
#include "sgx_trts.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "stbox/tsgx/ocall.h"
#include "ypc_t/ecommon/package.h"
#include <hpda/engine/engine.h>
#include <string.h>

#include "stbox/ebyte.h"
#include "stdlib.h"
#include "ypc_t/analyzer/extra_data_source.h"
#include <stdio.h> //vsnprintf

namespace ypc {

class parser_wrapper_base {
public:
  struct extra_data_source_group {
    std::string name;
    std::vector<std::shared_ptr<extra_data_source>> data_sources;
  };

  parser_wrapper_base();
  parser_wrapper_base(const parser_wrapper_base &) = delete;
  parser_wrapper_base &operator=(const parser_wrapper_base &) = delete;

  virtual ~parser_wrapper_base();
  virtual uint32_t begin_parse_data_item();
  virtual uint32_t parse_data_item(const uint8_t *sealed_data, uint32_t len);
  virtual uint32_t end_parse_data_item();

  virtual inline uint32_t get_encrypted_result_size() const {
    return m_encrypted_result_str.size();
  }
  virtual uint32_t
  get_encrypted_result_and_signature(uint8_t *encrypted_res, uint32_t res_size,
                                     uint8_t *result_sig, uint32_t sig_size,
                                     uint8_t *cost_sig, uint32_t cost_sig_size);

  virtual uint32_t add_block_parse_result(uint16_t block_index,
                                          uint8_t *block_result,
                                          uint32_t res_size, uint8_t *data_hash,
                                          uint32_t hash_size, uint8_t *sig,
                                          uint32_t sig_size);

  virtual uint32_t merge_parse_result(const uint8_t *encrypted_param,
                                      uint32_t len);

  virtual bool user_def_block_result_merge(
      const std::vector<stbox::bytes> &block_results) = 0;

  virtual uint32_t get_result_encrypt_key_size() = 0;
  virtual uint32_t get_result_encrypt_key(uint8_t *key, uint32_t key_size) = 0;

  virtual utc::parser_type_t get_parser_type() = 0;

  inline bool need_continue() { return m_continue; }

  inline void set_enclave_hash(const uint8_t *hash, uint32_t hash_size) {
    m_enclave_hash = stbox::bytes(hash, hash_size);
  }

  uint32_t set_extra_data(const uint8_t *extra_data, uint32_t in_size);

protected:
  uint32_t request_private_key();
  uint32_t decrypt_param(const uint8_t *encrypted_param, uint32_t len);

  uint32_t request_extra_data_usage();

protected:
  std::unique_ptr<stbox::dh_session_initiator> m_datahub_session;
  std::unique_ptr<stbox::dh_session_initiator> m_keymgr_session;

  stbox::bytes m_enclave_hash;
  stbox::bytes m_response_str;
  stbox::bytes m_result_str;
  stbox::bytes m_private_key;
  stbox::bytes m_param;
  stbox::bytes m_pkey4v;
  stbox::bytes m_encrypted_param;
  uint64_t m_cost_gas;
  stbox::bytes m_encrypted_result_str;
  stbox::bytes m_cost_signature_str;
  stbox::bytes m_result_signature_str;

  ypc::nt<stbox::bytes>::extra_data_package_t m_extra_data;
  std::vector<extra_data_source_group> m_extra_data_sources;

  //! for merge block result

  struct block_meta_t {
    stbox::bytes encrypted_result;
    stbox::bytes data_hash;
    stbox::bytes sig;
  };
  std::unordered_map<uint16_t, block_meta_t> m_block_results;
  bool m_continue;
  hpda::engine m_engine;
};

using extra_data_source_group = parser_wrapper_base::extra_data_source_group;

} // namespace ypc
