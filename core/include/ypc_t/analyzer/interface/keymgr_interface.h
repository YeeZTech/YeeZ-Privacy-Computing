#pragma once
#include "common/limits.h"
#include "corecommon/package.h"
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

template <typename Crypto>
class keymgr_interface : virtual public enclave_hash_var,
                         virtual public keymgr_var,
                         virtual public request_key_var<true> {
  typedef Crypto ecc;

protected:
  uint32_t request_private_key_for_public_key(const stbox::bytes &pubkey,
                                              stbox::bytes &private_key) {
    stbox::bytes request_msg = ypc::make_bytes<stbox::bytes>::for_package<
        request_skey_from_pkey_pkg_t, nt<stbox::bytes>::pkey>(pubkey);

    auto status = keymgr_var::m_keymgr_session->send_request_recv_response(
        (char *)request_msg.data(), request_msg.size(),
        utc::max_keymgr_response_buf_size, private_key);

    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for m_keymgr_session->send_request_recv_response: "
                 << stbox::status_string(status);
      return status;
    }
    stbox::bytes check_pkey;
    status = (stbox::stx_status)ecc::generate_pkey_from_skey(private_key,
                                                             check_pkey);
    if (status) {
      LOG(ERROR) << "error for generate_secp256k1_pkey_from_skey: " << status;
      return status;
    }
    if (pubkey != check_pkey) {
      LOG(ERROR) << "check failed, invalid private key";
      return stbox::stx_status::kmgr_session_inconsistency_pkey_skey;
    }
    return status;
  }
};
} // namespace internal
} // namespace ypc
