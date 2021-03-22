#pragma once
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_session.h"

namespace stbox {
class dh_session_initiator : public dh_session {
public:
  typedef ocall<uint32_t(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id)>
      session_request_func_t;

  typedef ocall<uint32_t(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
                         uint32_t session_id)>
      exchange_report_func_t;

  typedef ocall<uint32_t(uint32_t session_id)> end_session_func_t;

  dh_session_initiator(const session_request_func_t &ocall_session_request,
                       const exchange_report_func_t &ocall_exchange_report,
                       const send_request_func_t &ocall_send_request,
                       const end_session_func_t &ocall_end_session);

  stx_status create_session();

  stx_status close_session();


protected:
  // Ocall to request for a session with the destination enclave and obtain
  // session id and Message 1 if successful
  session_request_func_t m_session_request_ocall;
  exchange_report_func_t m_exchange_report_ocall;
  end_session_func_t m_end_session_ocall;
};
} // namespace stbox
