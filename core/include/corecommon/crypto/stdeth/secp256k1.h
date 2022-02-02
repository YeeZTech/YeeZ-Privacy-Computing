#pragma once
#include <cstdint>

namespace ypc {
namespace crypto {
struct secp256k1 {
  inline static uint32_t get_private_key_size() { return 32; }
  inline static uint32_t get_public_key_size() { return 64; }
  static uint32_t gen_private_key(uint32_t skey_size, uint8_t *skey);

  static uint32_t generate_pkey_from_skey(const uint8_t *skey,
                                          uint32_t skey_size, uint8_t *pkey,
                                          uint32_t pkey_size);

  static uint32_t get_signature_size();

  static uint32_t sign_message(const uint8_t *skey, uint32_t skey_size,
                               const uint8_t *data, uint32_t data_size,
                               uint8_t *sig, uint32_t sig_size);

  static uint32_t verify_signature(const uint8_t *data, uint32_t data_size,
                                   const uint8_t *sig, uint32_t sig_size,
                                   const uint8_t *public_key,
                                   uint32_t pkey_size);
};
} // namespace crypto
} // namespace ypc
