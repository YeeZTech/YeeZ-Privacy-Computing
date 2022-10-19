#pragma once

struct secp256k1_ecdh_sgx128 {
  inline static uint32_t get_ecdh_shared_key_size() { return 16; }
  static uint32_t ecdh_shared_key(const uint8_t *skey, uint32_t skey_size,
                                  const uint8_t *public_key, uint32_t pkey_size,
                                  uint8_t *shared_key,
                                  uint32_t shared_key_size);
};
