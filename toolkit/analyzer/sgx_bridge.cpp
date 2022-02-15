#include "sgx_bridge.h"
#include "ypc/memref.h"

using stx_status = stbox::stx_status;
std::shared_ptr<parser> g_parser;

extern "C" {
uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id);
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size);
uint32_t km_end_session_ocall(uint32_t session_id);

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len);
void free_data_batch(uint8_t *data);
}

uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1,
                                  uint32_t *session_id) {
  return g_parser->keymgr()->session_request(dh_msg1, session_id);
}
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id) {
  return g_parser->keymgr()->exchange_report(dh_msg2, dh_msg3, session_id);
}
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size) {
  return g_parser->keymgr()->generate_response(req_message, req_message_size,
                                               max_payload_size, resp_message,
                                               resp_message_size, session_id);
}
uint32_t km_end_session_ocall(uint32_t session_id) {
  return g_parser->keymgr()->end_session(session_id);
}

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len) {
  return g_parser->next_data_batch(data_hash, hash_size, data, len);
}
void free_data_batch(uint8_t *data) { g_parser->free_data_batch(data); }

