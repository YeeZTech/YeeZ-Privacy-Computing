#include "ypc_t/ecommon/signer_verify.h"
#include "stbox/tsgx/log.h"

// This is hardcoded responder enclave's MRSIGNER for demonstration purpose. The
// content aligns to responder enclave's signing key
sgx_measurement_t g_responder_mrsigner = {
    {0x83, 0xd7, 0x19, 0xe7, 0x7d, 0xea, 0xca, 0x14, 0x70, 0xf6, 0xba,
     0xf6, 0x2a, 0x4d, 0x77, 0x43, 0x03, 0xc8, 0x99, 0xdb, 0x69, 0x02,
     0x0f, 0x9c, 0x70, 0xee, 0x1d, 0xfc, 0x08, 0xc7, 0xce, 0x9e}};
#define RESPONDER_PRODID 1

namespace ypc {
bool is_certified_signer(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
#if 0
  if (memcmp((uint8_t *)&peer_enclave_identity->mr_signer,
             (uint8_t *)&g_responder_mrsigner, sizeof(sgx_measurement_t))) {
    LOG(ERROR) << "not a trusted signer";
    return false;
  }
#endif

#if 0
  if (peer_enclave_identity->isv_prod_id != RESPONDER_PRODID ||
      !(peer_enclave_identity->attributes.flags & SGX_FLAGS_INITTED)) {
    LOG(ERROR) << "wrong prod id, or SGX_FLAGS_INITTED error";
    return false;
  }
#endif

  // check the enclave isn't loaded in enclave debug mode, except that the
  // project is built for debug purpose
#if defined(NDEBUG)
  if (peer_enclave_identity->attributes.flags & SGX_FLAGS_DEBUG) {
    LOG(ERROR) << "shouldn't loaded ad DEBUG";
    return false;
  }
#endif
  return true;
}
} // namespace ypc
