#pragma once
#include "stbox/tsgx/channel/dh_session.h"
#include <map>

namespace stbox {
class dh_session_responder_instance : public dh_session {
public:
  stx_status session_request(sgx_dh_msg1_t *dh_msg1, uint32_t session_id);
  stx_status exchange_report(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3);
  stx_status end_session();
};

class dh_session_responder {
public:
  // Handle the request from Source Enclave for a session
  stx_status session_request(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);

  // Verify Message 2, generate Message3 and exchange Message 3 with Source
  // Enclave
  stx_status exchange_report(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
                             uint32_t session_id);

  stx_status generate_response(secure_message_t *req_message,
                               size_t req_message_size,
                               const dh_session::handler_func_t &handler,
                               size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size, uint32_t session_id);

  stx_status end_session(uint32_t session_id);

  using send_request_func_t = dh_session::send_request_func_t;
  inline void set_send_request_func(const send_request_func_t &func) {
    m_send_request_ocall = func;
  }
  using verify_peer_func_t = dh_session::verify_peer_func_t;
  inline void set_verify_peer(const verify_peer_func_t &func) {
    m_verify_peer_enclave_trust = func;
  }

  using all_sessions_t =
      std::map<uint32_t, std::shared_ptr<dh_session_responder_instance>>;
  inline const all_sessions_t &all_sessions() const { return m_all_sessions; }

protected:
  stx_status generate_session_id(uint32_t &session_id);

protected:
  std::map<uint32_t, std::shared_ptr<dh_session_responder_instance>>
      m_all_sessions;
  uint32_t m_next_session_id;
  send_request_func_t m_send_request_ocall;
  verify_peer_func_t m_verify_peer_enclave_trust;
};
} // namespace stbox
