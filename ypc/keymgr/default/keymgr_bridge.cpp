#include "ypc/keymgr/default/keymgr_bridge.h"
#include "ekeymgr_u.h"
#include <glog/logging.h>
#include <memory>

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
}

namespace ypc {
std::shared_ptr<ypc::keymgr_parser> g_keymgr_parser;
void init_sgx_keymgr(std::shared_ptr<ypc::keymgr_sgx_module> &keymgr) {
  g_keymgr_parser = std::make_shared<ypc::keymgr_parser>(keymgr);
}
void shutdown_sgx_keymgr() {}
} // namespace ypc

uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1,
                                  uint32_t *session_id) {
  return ypc::g_keymgr_parser->keymgr()->session_request(dh_msg1, session_id);
}
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id) {
  return ypc::g_keymgr_parser->keymgr()->exchange_report(dh_msg2, dh_msg3,
                                                         session_id);
}
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size) {
  return ypc::g_keymgr_parser->keymgr()->generate_response(
      req_message, req_message_size, max_payload_size, resp_message,
      resp_message_size, session_id);
}
uint32_t km_end_session_ocall(uint32_t session_id) {
  return ypc::g_keymgr_parser->keymgr()->end_session(session_id);
}
