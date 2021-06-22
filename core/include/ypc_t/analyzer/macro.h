#include "ypc_t/analyzer/parser_wrapper.h"
#include "ypc_t/analyzer/parser_wrapper_for_offchain.h"

#define YPC_PARSER_IMPL(pw)                                                    \
  uint32_t begin_parse_data_item() {                                           \
    stbox::bytes hash(get_enclave_hash_size());                                \
    get_enclave_hash(hash.data(), hash.size());                                \
                                                                               \
    pw.set_enclave_hash(hash.data(), hash.size());                             \
                                                                               \
    return pw.begin_parse_data_item();                                         \
  }                                                                            \
  uint32_t parse_data_item(uint8_t *sealed_data, uint32_t len) {               \
    pw.set_item_parser(ypc::ecall_parse_item_data);                            \
    return pw.parse_data_item(sealed_data, len);                               \
  }                                                                            \
  uint32_t end_parse_data_item() { return pw.end_parse_data_item(); }          \
                                                                               \
  uint32_t get_enclave_hash_size() { return SGX_HASH_SIZE; }                   \
  uint32_t get_enclave_hash(uint8_t *hash, uint32_t hash_size) {               \
    sgx_report_data_t data;                                                    \
    uint32_t ret = 0;                                                          \
    memset(&data.d, 0xff, sizeof data.d);                                      \
    sgx_report_t report;                                                       \
    ret = sgx_create_report(NULL, &data, &report);                             \
    if (ret != SGX_SUCCESS) {                                                  \
      return ret;                                                              \
    }                                                                          \
    memcpy(hash, report.body.mr_enclave.m, hash_size);                         \
    return ret;                                                                \
  }                                                                            \
                                                                               \
  uint32_t get_encrypted_result_size() {                                       \
    return pw.get_encrypted_result_size();                                     \
  }                                                                            \
  uint32_t get_encrypted_result_and_signature(                                 \
      uint8_t *encrypted_res, uint32_t res_size, uint8_t *result_sig,          \
      uint32_t sig_size) {                                                     \
    return pw.get_encrypted_result_and_signature(encrypted_res, res_size,      \
                                                 result_sig, sig_size);        \
  }                                                                            \
                                                                               \
  uint32_t add_block_parse_result(uint16_t block_index, uint8_t *block_result, \
                                  uint32_t res_size, uint8_t *data_hash,       \
                                  uint32_t hash_size, uint8_t *sig,            \
                                  uint32_t sig_size) {                         \
    return pw.add_block_parse_result(block_index, block_result, res_size,      \
                                     data_hash, hash_size, sig, sig_size);     \
  }                                                                            \
                                                                               \
  uint32_t merge_parse_result(uint8_t *encrypted_param, uint32_t len) {        \
    return pw.merge_parse_result(encrypted_param, len);                        \
  }                                                                            \
                                                                               \
  uint32_t need_continue() { return pw.need_continue(); }                      \
                                                                               \
  uint32_t get_data_hash_size() { return pw.data_hash().size(); }              \
  uint32_t get_data_hash(uint8_t *hash, uint32_t hash_size) {                  \
    auto t = pw.data_hash();                                                   \
    uint32_t ret = SGX_SUCCESS;                                                \
    memcpy(hash, t.data(), t.size());                                          \
    return ret;                                                                \
  }                                                                            \
  uint32_t get_result_encrypt_key_size() {                                     \
    return pw.get_result_encrypt_key_size();                                   \
  }                                                                            \
  uint32_t get_result_encrypt_key(uint8_t *key, uint32_t key_size) {           \
    return pw.get_result_encrypt_key(key, key_size);                           \
  }
