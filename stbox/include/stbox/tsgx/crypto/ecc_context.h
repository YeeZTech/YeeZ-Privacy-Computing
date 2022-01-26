#pragma once
#include "stbox/ebyte.h"
//#include "stbox/tsgx/secp256k1/secp256k1.h"
//#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
//#include "stbox/tsgx/secp256k1/secp256k1_preallocated.h"
//#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"
#include "common/endian.h"
#include "stbox/tsgx/crypto/ecc.h"

namespace stbox {
namespace crypto {
template <typename Curve> class ecc_context {
public:
  typedef Curve curve_t;

  ecc_context() {}
  virtual ~ecc_context() {}

  const bytes &skey() {
    if (m_skey.empty()) {
      ecc<curve_t>::gen_private_key(m_skey);
      ecc<curve_t>::generate_pkey_from_skey(m_skey, m_pkey_big);
      m_pkey_little = m_pkey_big;
      ::ypc::utc::change_pubkey_endian(m_pkey_little);
    }
    return m_skey;
  }

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
};
} // namespace crypto
} // namespace stbox
