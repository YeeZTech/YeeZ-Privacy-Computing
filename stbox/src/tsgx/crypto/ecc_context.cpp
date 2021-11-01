#include "stbox/tsgx/crypto/ecc_context.h"
#include "common/endian.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/log.h"

#define SECP256K1_PRIVATE_KEY_SIZE 32

namespace stbox {
namespace crypto {

ecc_context::ecc_context() {
  m_ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                   SECP256K1_CONTEXT_SIGN);
}
const bytes &ecc_context::skey() {
  if (m_skey.empty()) {
    uint32_t skey_size = SECP256K1_PRIVATE_KEY_SIZE;
    m_skey = bytes(skey_size);
    gen_secp256k1_skey(skey_size, m_skey.data());

    m_pkey_big = bytes(sizeof(secp256k1_pubkey));

    generate_secp256k1_pkey_from_skey(m_skey.data(), m_pkey_big.data(),
                                      m_pkey_big.size());

    m_pkey_little = m_pkey_big;
    ::ypc::utc::change_pubkey_endian(m_pkey_little);
  }
  return m_skey;
}
ecc_context::~ecc_context() { secp256k1_context_destroy(m_ctx); }
} // namespace crypto
} // namespace stbox
