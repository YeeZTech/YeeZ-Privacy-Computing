#pragma once
#include "stbox/ebyte.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include "stbox/tsgx/secp256k1/secp256k1_preallocated.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"

namespace stbox {
namespace crypto {
class ecc_context {
public:
  ecc_context();
  virtual ~ecc_context();
  inline secp256k1_context *ctx() { return m_ctx; }

  const bytes &skey();

  inline const bytes &pkey_little_endian() {
    skey();
    return m_pkey_little;
  }
  inline const bytes &pkey_big_endian() {
    skey();
    return m_pkey_big;
  }

protected:
  bytes m_skey;
  bytes m_pkey_big;
  bytes m_pkey_little;
  secp256k1_context *m_ctx;
};
} // namespace crypto
} // namespace stbox
