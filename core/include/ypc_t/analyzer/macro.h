#include "sgx_utils.h"
#include "ypc_t/analyzer/helper/ecall_impl_helper.h"

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
  uint32_t get_analyze_result_size() { return pw.get_analyze_result_size(); }  \
  uint32_t get_analyze_result(uint8_t *res, uint32_t res_size) {               \
    return pw.get_analyze_result(res, res_size);                               \
  }                                                                            \
  uint32_t init_data_source(const uint8_t *data_source_info, uint32_t len) {   \
    return call_init_data_source_helper<decltype(pw)>::call(                   \
        pw, data_source_info, len);                                            \
  }                                                                            \
  uint32_t init_model(const uint8_t *model, uint32_t len) {                    \
    return call_init_model_helper<decltype(pw)>::call(pw, model, len);         \
  }                                                                            \
  uint32_t get_parser_type() { return pw.get_parser_type(); }

#define YPC_PARSER_IMPL(...)                                                   \
  JOIN(YPC_PARSER_IMPL_, PP_NARG(__VA_ARGS__))(__VA_ARGS__)

#define YPC_PARSER_IMPL_1(pw)                                                  \
  COMMON_PARSER_IMPL(pw)                                                       \
  uint32_t parse_data_item(const uint8_t *input_param, uint32_t len) {         \
    return pw.parse_data_item(input_param, len);                               \
  }
