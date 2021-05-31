#include "stbox/tsgx/channel/dh_session_responder.h"
#include "stbox/tsgx/log.h"
namespace stbox {

stx_status
dh_session_responder_instance::session_request(sgx_dh_msg1_t *dh_msg1,
                                               uint32_t session_id) {
  // dh_session_t session_info;
  sgx_dh_session_t sgx_dh_session;
  sgx_status_t status = SGX_SUCCESS;

  // Intialize the session as a session responder
  status = sgx_dh_init_session(SGX_DH_SESSION_RESPONDER, &sgx_dh_session);
  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }

  m_status = session_status::in_progress;

  // Generate Message1 that will be returned to Source Enclave
  status = sgx_dh_responder_gen_msg1((sgx_dh_msg1_t *)dh_msg1, &sgx_dh_session);
  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }
  memcpy(&m_state.in_progress.dh_session, &sgx_dh_session,
         sizeof(sgx_dh_session_t));
  return static_cast<stx_status>(status);
}

stx_status
dh_session_responder_instance::exchange_report(sgx_dh_msg2_t *dh_msg2,
                                               sgx_dh_msg3_t *dh_msg3) {
  sgx_key_128bit_t dh_aek; // Session key
  stx_status status = stx_status::success;
  sgx_dh_session_enclave_identity_t initiator_identity;

  dh_msg3->msg3_body.additional_prop_length = 0;
  // Process message 2 from source enclave and obtain message 3
  sgx_status_t se_ret = sgx_dh_responder_proc_msg2(
      dh_msg2, dh_msg3, &m_state.in_progress.dh_session, &dh_aek,
      &initiator_identity);
  if (se_ret != SGX_SUCCESS) {
    end_session();
    return static_cast<stx_status>(se_ret);
  }
  if (m_verify_peer_enclave_trust(&initiator_identity) != stx_status::success) {
    end_session();
    return stx_status::invalid_session;
  }
  m_status = session_status::active;
  m_state.active.counter = 0;
  memcpy(m_state.active.AEK, &dh_aek, sizeof(sgx_key_128bit_t));
  memset(&dh_aek, 0, sizeof(sgx_key_128bit_t));
  memcpy((void *)&m_peer_identity, &initiator_identity,
         sizeof(initiator_identity));

  return stx_status::success;
}

stx_status dh_session_responder_instance::end_session() {
  // Seems nothing to do here
  return stx_status::success;
}

stx_status dh_session_responder::session_request(sgx_dh_msg1_t *dh_msg1,
                                                 uint32_t *session_id) {

  if (!session_id || !dh_msg1) {
    return stx_status::invalid_parameter_error;
  }
  *session_id = m_next_session_id;
  m_next_session_id++;
  std::shared_ptr<dh_session_responder_instance> p(
      new dh_session_responder_instance());
  p->set_send_request_func(m_send_request_ocall);
  p->set_verify_peer(m_verify_peer_enclave_trust);

  auto status = p->session_request(dh_msg1, *session_id);
  if (status == stx_status::success) {
    m_all_sessions.insert(std::make_pair(*session_id, p));
  }
  return status;
}

stx_status dh_session_responder::exchange_report(sgx_dh_msg2_t *dh_msg2,
                                                 sgx_dh_msg3_t *dh_msg3,
                                                 uint32_t session_id) {
  auto it = m_all_sessions.find(session_id);
  if (it == m_all_sessions.end()) {
    return stx_status::invalid_session;
  }
  if (!it->second->is_in_progress()) {
    return stx_status::invalid_session;
  }
  if (!dh_msg2 || !dh_msg3) {
    return stx_status::invalid_parameter_error;
  }

  return it->second->exchange_report(dh_msg2, dh_msg3);
}
stx_status dh_session_responder::generate_response(
    secure_message_t *req_message, size_t req_message_size,
    const dh_session::handler_func_t &handler, size_t max_payload_size,
    secure_message_t *resp_message, size_t resp_message_size,
    uint32_t session_id) {
  auto it = m_all_sessions.find(session_id);
  if (it == m_all_sessions.end()) {
    return stx_status::invalid_session;
  }
  return it->second->generate_response(req_message, req_message_size, handler,
                                       max_payload_size, resp_message,
                                       resp_message_size);
}
stx_status dh_session_responder::end_session(uint32_t session_id) {
  auto it = m_all_sessions.find(session_id);
  if (it == m_all_sessions.end()) {
    return stx_status::invalid_session;
  }
  auto ret = it->second->end_session();
  m_all_sessions.erase(it);
  return stx_status::success;
}
} // namespace stbox
