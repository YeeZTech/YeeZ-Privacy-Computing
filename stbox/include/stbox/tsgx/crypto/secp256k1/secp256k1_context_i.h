#pragma once
#include "stbox/tsgx/crypto/ecc_context.h"
#include "stbox/tsgx/crypto/ecc_curves.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
namespace stbox {
namespace crypto {
namespace internal {
class secp256k1_context_i : public ecc_context<secp256k1> {
public:
  secp256k1_context_i();
  virtual ~secp256k1_context_i();
  inline secp256k1_context *ctx() { return m_ctx; }

protected:
  secp256k1_context *m_ctx;
};

} // namespace internal
} // namespace crypto
} // namespace stbox
