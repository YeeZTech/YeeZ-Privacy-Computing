#include "ypc_t/analyzer/parser_wrapper.h"
#include "ypc_t/analyzer/parser_wrapper_for_offchain.h"

#define COMMON_PARSER_IMPL(pw)                                                 \
  uint32_t begin_parse_data_item() {                                           \
    stbox::bytes hash(get_enclave_hash_size());                                \
    get_enclave_hash(hash.data(), hash.size());                                \
                                                                               \
    pw.set_enclave_hash(hash.data(), hash.size());                             \
                                                                               \
    return pw.begin_parse_data_item();                                         \
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
  uint32_t get_analyze_result_size() {                                         \
    using ntt = ypc::nt<stbox::bytes>;                                         \
    ntt::ypc_result_package_t pkg;                                             \
    auto ret = pw.get_analyze_result(pkg);                                     \
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);                \
    pkg.arch(lm);                                                              \
    return lm.get_length();                                                    \
  }                                                                            \
  uint32_t get_analyze_result(uint8_t *res, uint32_t res_size) {               \
    using ntt = ypc::nt<stbox::bytes>;                                         \
    ntt::ypc_result_package_t pkg;                                             \
    auto ret = pw.get_analyze_result(pkg);                                     \
                                                                               \
    ff::net::marshaler ld((char *)res, res_size,                               \
                          ff::net::marshaler::serializer);                     \
    pkg.arch(ld);                                                              \
    return ret;                                                                \
  }                                                                            \
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
  uint32_t get_parser_type() { return pw.get_parser_type(); }                  \
  uint32_t set_extra_data(uint8_t *extra_data, uint32_t in_size) {             \
    return pw.set_extra_data(extra_data, in_size);                             \
  }

#define YPC_PARSER_IMPL(...)                                                   \
  JOIN(YPC_PARSER_IMPL_, PP_NARG(__VA_ARGS__))(__VA_ARGS__)

#define YPC_PARSER_IMPL_1(pw)                                                  \
  COMMON_PARSER_IMPL(pw)                                                       \
  uint32_t parse_data_item(uint8_t *sealed_data, uint32_t len) {               \
    return pw.parse_data_item(sealed_data, len);                               \
  }
#define YPC_PARSER_IMPL_2(pw, parser_item_func)                                \
  COMMON_PARSER_IMPL(pw)                                                       \
  uint32_t parse_data_item(uint8_t *sealed_data, uint32_t len) {               \
    pw.set_item_parser(parser_item_func);                                      \
    return pw.parse_data_item(sealed_data, len);                               \
  }

