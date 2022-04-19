#include "ypc_t/analyzer/internal/keymgr_session.h"
#include "yaenclave_t.h"
#include "ypc_t/ecommon/signer_verify.h"

namespace ypc {
namespace internal {

stbox::stx_status km_verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity) {
    LOG(ERROR) << "verify key manager enclave failed";
    return stbox::stx_status::invalid_parameter_error;
  }

  return stbox::stx_status::success;
}
uint32_t keymgr_session::init_keymgr_session() {
  if (keymgr_var::m_keymgr_session) {
    return stbox::stx_status::success;
  }

  keymgr_var::m_keymgr_session.reset(new stbox::dh_session_initiator(
      stbox::ocall_cast<uint32_t>(km_session_request_ocall),
      stbox::ocall_cast<uint32_t>(km_exchange_report_ocall),
      stbox::ocall_cast<uint32_t>(km_send_request_ocall),
      stbox::ocall_cast<uint32_t>(km_end_session_ocall)));
  keymgr_var::m_keymgr_session->set_verify_peer(km_verify_peer_enclave_trust);

  LOG(INFO) << "done init keymgr session";

  auto t = keymgr_var::m_keymgr_session->create_session();
  LOG(INFO) << "done create keymgr session";
  return t;
}
uint32_t keymgr_session::close_keymgr_session() {
  return keymgr_var::m_keymgr_session->close_session();
}

} // namespace internal
} // namespace ypc
