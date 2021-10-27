#pragma once
#include "stbox/ebyte.h"
#include "stbox/scope_guard.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_cdef.h"
#include "stbox/tsgx/ocall.h"
#include <sgx_attributes.h>
#include <sgx_ecp_types.h>
#include <sgx_eid.h>
#include <sgx_key.h>
#include <sgx_report.h>
//#include <sgx_trts.h>

#define TAG_SIZE 16

namespace stbox {
class dh_session {
public:
  enum session_status { in_progress, active, closed };

  dh_session();
  dh_session(const dh_session &) = delete;
  dh_session &operator=(const dh_session &) = delete;
  virtual ~dh_session();

  typedef ocall<uint32_t(uint32_t session_id, secure_message_t *req_message,
                         size_t req_message_len, size_t max_out_buff_size,
                         secure_message_t *resp_message,
                         size_t resp_message_len)>
      send_request_func_t;

  typedef std::function<stx_status(sgx_dh_session_enclave_identity_t *)>
      verify_peer_func_t;

  template <typename FT> inline void set_send_request_func(const FT &f) {
    set_send_request_func(send_request_func_t(f));
  }

  inline void set_send_request_func(const send_request_func_t &func) {
    m_send_request_ocall = func;
    m_send_request_ocall_set = true;
  }
  inline void set_verify_peer(const verify_peer_func_t &func) {
    m_verify_peer_enclave_trust = func;
    m_verify_peer_enclave_trust_set = true;
  }
  // TODO: maybe send_request_recv_response is only for initiator,
  // while generate_response is only for responder.
  // If so, we move these two methods
  virtual stx_status send_request_recv_response(const char *inp_buff,
                                                size_t inp_buff_len,
                                                size_t max_out_buff_size,
                                                bytes &out_buff);

  typedef std::function<bytes(const uint8_t *data, size_t data_len,
                              dh_session *context)>
      handler_func_t;

  virtual stx_status
  generate_response(secure_message_t *req_message, size_t req_message_size,
                    const handler_func_t &handler, size_t max_payload_size,
                    secure_message_t *resp_message, size_t resp_message_size);

  bool is_in_progress() const {
    return m_status == session_status::in_progress;
  }
  inline const sgx_dh_session_enclave_identity_t &self_identity() const {
    return m_self_identity;
  }
  inline const sgx_dh_session_enclave_identity_t &peer_identity() const {
    return m_peer_identity;
  }

protected:
  union {
    struct {
      sgx_dh_session_t dh_session;
    } in_progress;

    struct {
      sgx_key_128bit_t AEK; // Session Key
      uint32_t counter;     // Used to store Message Sequence Number
    } active;
  } m_state;
  uint32_t m_session_id; // Identifies the current session
  session_status m_status; // Indicates session is in progress, active or closed
  send_request_func_t m_send_request_ocall;
  bool m_send_request_ocall_set;
  verify_peer_func_t m_verify_peer_enclave_trust;
  bool m_verify_peer_enclave_trust_set;

  sgx_dh_session_enclave_identity_t m_self_identity;
  sgx_dh_session_enclave_identity_t m_peer_identity;
};

} // namespace stbox
