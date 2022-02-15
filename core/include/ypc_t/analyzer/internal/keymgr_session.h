#pragma once
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "stbox/tsgx/ocall.h"
#include "ypc_t/analyzer/var/enclave_hash_var.h"
#include "ypc_t/analyzer/var/keymgr_var.h"
#include "ypc_t/analyzer/var/request_key_var.h"
#include "ypc_t/ecommon/package.h"

namespace ypc {
namespace internal {

stbox::stx_status km_verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity);

class keymgr_session : virtual public enclave_hash_var,
                       virtual public keymgr_var {
protected:
  uint32_t init_keymgr_session();
  uint32_t close_keymgr_session();
};
} // namespace internal
} // namespace ypc
