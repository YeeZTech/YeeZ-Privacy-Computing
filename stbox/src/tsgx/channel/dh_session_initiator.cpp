#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "stbox/tsgx/log.h"

namespace stbox {

dh_session_initiator::dh_session_initiator(
    const session_request_func_t &ocall_session_request,
    const exchange_report_func_t &ocall_exchange_report,
    const send_request_func_t &ocall_send_request,
    const end_session_func_t &ocall_end_session)
    : dh_session(), m_session_request_ocall(ocall_session_request),
      m_exchange_report_ocall(ocall_exchange_report),
      m_end_session_ocall(ocall_end_session) {
  set_send_request_func(ocall_send_request);
}

stx_status dh_session_initiator::create_session() {
  sgx_dh_msg1_t dh_msg1;   // Diffie-Hellman Message 1
  sgx_key_128bit_t dh_aek; // Session Key
  sgx_dh_msg2_t dh_msg2;   // Diffie-Hellman Message 2
  sgx_dh_msg3_t dh_msg3;   // Diffie-Hellman Message 3
  uint32_t session_id;
  uint32_t retstatus;
  sgx_status_t status = SGX_SUCCESS;
  sgx_dh_session_t sgx_dh_session;
  sgx_dh_session_enclave_identity_t responder_identity;


  memset(&dh_aek, 0, sizeof(sgx_key_128bit_t));
  memset(&dh_msg1, 0, sizeof(sgx_dh_msg1_t));
  memset(&dh_msg2, 0, sizeof(sgx_dh_msg2_t));
  memset(&dh_msg3, 0, sizeof(sgx_dh_msg3_t));

  // Intialize the session as a session initiator
  status = sgx_dh_init_session(SGX_DH_SESSION_INITIATOR, &sgx_dh_session);
  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }

  // Ocall to request for a session with the destination enclave and obtain
  // session id and Message 1 if successful
  retstatus = m_session_request_ocall(&dh_msg1, &session_id);
  if (static_cast<stx_status>(retstatus) != stx_status::success) {
    return static_cast<stx_status>(retstatus);
  }
  // Process the message 1 obtained from desination enclave and generate message
  // 2
  status = sgx_dh_initiator_proc_msg1(&dh_msg1, &dh_msg2, &sgx_dh_session);
  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }

  // Send Message 2 to Destination Enclave and get Message 3 in return
  retstatus = m_exchange_report_ocall(&dh_msg2, &dh_msg3, session_id);
  if (static_cast<stx_status>(retstatus) != stx_status::success) {
    return static_cast<stx_status>(retstatus);
  }

  // Process Message 3 obtained from the destination enclave
  status = sgx_dh_initiator_proc_msg3(&dh_msg3, &sgx_dh_session, &dh_aek,
                                      &responder_identity);
  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }
  // Verify the identity of the destination enclave
  if (m_verify_peer_enclave_trust(&responder_identity) != stx_status::success) {
    return stx_status::invalid_session;
  }
  memcpy((void *)&m_peer_identity, &responder_identity,
         sizeof(responder_identity));

  memcpy(m_state.active.AEK, &dh_aek, sizeof(sgx_key_128bit_t));
  m_session_id = session_id;
  m_state.active.counter = 0;
  m_status = session_status::active;
  // memset(&dh_aek, 0, sizeof(sgx_key_128bit_t));
  return static_cast<stx_status>(status);
}

stx_status dh_session_initiator::close_session() {
  sgx_status_t status;
  uint32_t retstatus;

  // Ocall to ask the destination enclave to end the session
  retstatus = m_end_session_ocall(m_session_id);
  if (static_cast<stx_status>(retstatus) != stx_status::success) {
    return static_cast<stx_status>(retstatus);
  }
  return stx_status::success;
}
} // namespace stbox
