#pragma once
#include "sgx_eid.h"
#include "sgx_error.h"
#include "ypc/byte.h"
#include <stbox/usgx/sgx_module.h>
#include <string>
#include <ypc/ref.h>

using stx_status = stbox::stx_status;
class parser_sgx_module : public stbox::sgx_module {
public:
  parser_sgx_module(const char *mod_path);
  virtual ~parser_sgx_module();

  uint32_t begin_parse_data_item();
  uint32_t parse_data_item(const char *data, size_t len);
  uint32_t end_parse_data_item();

  uint32_t get_enclave_hash(ypc::bref &enclave_hash);
  uint32_t get_encrypted_result_and_signature(ypc::bref &encrypted_res,
                                              ypc::bref &result_sig);

  uint32_t get_data_hash(ypc::bref &data_hash);

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

  bool need_continue();
};
