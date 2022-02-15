#pragma once
#include "common/parser_type.h"
#include "corecommon/nt_cols.h"
#include "sgx_eid.h"
#include "sgx_error.h"
#include "ypc/byte.h"
#include <stbox/usgx/sgx_module.h>
#include <string>
#include <ypc/ref.h>

using stx_status = stbox::stx_status;
using parser_type_t = ypc::utc::parser_type_t;
namespace ypc {
class parser_sgx_module : public stbox::sgx_module {
public:
  parser_sgx_module(const char *mod_path);
  virtual ~parser_sgx_module();

  uint32_t begin_parse_data_item();
  uint32_t parse_data_item(const uint8_t *data, size_t len);
  uint32_t end_parse_data_item();

  uint32_t get_enclave_hash(ypc::bytes &enclave_hash);

  // uint32_t get_encrypted_result_hash(ypc::bref &hash);

  uint32_t get_analyze_result(ypc::bytes &res);


  uint32_t init_data_source(const ypc::bytes &info);
  uint32_t init_model(const ypc::bytes &info);
  uint32_t get_parser_type();

  /*
  uint32_t add_block_parse_result(uint16_t block_index,
                                  const uint8_t *block_result,
                                  uint32_t res_size, const uint8_t *data_hash,
                                  uint32_t hash_size, const uint8_t *sig,
                                  uint32_t sig_size);

  inline uint32_t add_block_parse_result(uint16_t block_index,
                                         const ypc::bytes &block_result,
                                         const ypc::bytes &data_hash,
                                         const ypc::bytes &sig) {
    return add_block_parse_result(block_index, block_result.data(),
                                  block_result.size(), data_hash.data(),
                                  data_hash.size(), sig.data(), sig.size());
  }

  uint32_t merge_parse_result(const uint8_t *encrypted_param, uint32_t len);
  inline uint32_t merge_parse_result(const ypc::bytes &encrypted_param) {
    return merge_parse_result(encrypted_param.data(), encrypted_param.size());
  }

  uint32_t set_extra_data(const uint8_t *extra_data, uint32_t in_size);

  bool need_continue();
  */
};
} // namespace ypc
