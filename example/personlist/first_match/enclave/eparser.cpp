#include "eparser_t.h"
#include "first_match_parser.h"
#include "sgx_plugin.h"
#include "sgx_utils.h"
#include "stbox/tsgx/log.h"
#include "ypc_t/analyzer/parser_wrapper.h"
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>

ypc::parser_wrapper<user_item_t, first_match_parser> pw;

uint32_t begin_parse_data_item() { return pw.begin_parse_data_item(); }
uint32_t parse_data_item(uint8_t *sealed_data, uint32_t len) {
  pw.set_item_parser(ypc::ecall_parse_item_data);
  return pw.parse_data_item(sealed_data, len);
}
uint32_t end_parse_data_item() { return pw.end_parse_data_item(); }

uint32_t get_enclave_hash_size() { return SGX_HASH_SIZE; }
uint32_t get_enclave_hash(uint8_t *hash, uint32_t hash_size) {
  sgx_report_data_t data;
  uint32_t ret = 0;
  memset(&data.d, 0xff, sizeof data.d); // random data
  sgx_report_t report;
  ret = sgx_create_report(NULL, &data, &report);
  if (ret != SGX_SUCCESS) {
    return ret;
  }
  memcpy(hash, report.body.mr_enclave.m, hash_size);
  return ret;
}

uint32_t add_block_parse_result(uint16_t block_index, uint8_t *block_result,
                                uint32_t res_size, uint8_t *data_hash,
                                uint32_t hash_size, uint8_t *sig,
                                uint32_t sig_size) {
  return pw.add_block_parse_result(block_index, block_result, res_size,
                                   data_hash, hash_size, sig, sig_size);
}

uint32_t merge_parse_result(uint8_t *encrypted_param, uint32_t len) {
  stbox::printf("xxxx");
  LOG(INFO) << "start merge result";
  return pw.merge_parse_result(encrypted_param, len);
}
uint32_t need_continue() { return pw.need_continue(); }

uint32_t get_encrypted_result_size() { return pw.get_encrypted_result_size(); }
uint32_t get_encrypted_result_and_signature(uint8_t *encrypted_res,
                                            uint32_t res_size,
                                            uint8_t *result_sig,
                                            uint32_t sig_size) {
  return pw.get_encrypted_result_and_signature(encrypted_res, res_size,
                                               result_sig, sig_size);
}

uint32_t get_data_hash_size() { return pw.data_hash().size(); }

uint32_t get_data_hash(uint8_t *hash, uint32_t hash_size) {
  auto t = pw.data_hash();
  uint32_t ret = SGX_SUCCESS;
  memcpy(hash, t.value(), t.size());
  return ret;
}
