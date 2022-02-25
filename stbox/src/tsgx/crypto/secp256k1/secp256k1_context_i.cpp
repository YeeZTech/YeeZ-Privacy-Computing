#include "stbox/tsgx/crypto/secp256k1/secp256k1_context_i.h"
#include "stbox/tsgx/crypto/ecc_context.h"
#include "stbox/tsgx/crypto/secp256k1/ecc_secp256k1.h"

#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include "stbox/tsgx/secp256k1/secp256k1_preallocated.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"

namespace stbox {
namespace crypto {
namespace internal {
secp256k1_context_i::secp256k1_context_i() : ecc_context<secp256k1>() {
  m_ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                   SECP256K1_CONTEXT_SIGN);
}

secp256k1_context_i::~secp256k1_context_i() {
  secp256k1_context_destroy(m_ctx);
}
} // namespace internal
} // namespace crypto
} // namespace stbox
