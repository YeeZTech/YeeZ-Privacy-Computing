#pragma once
#include "stbox/tsgx/crypto/ecc_curves.h"
#include "stbox/tsgx/crypto/raw_ecc.h"

namespace stbox {
namespace crypto {
namespace internal {
class secp256k1_context_i;
} // namespace internal
template <> struct raw_ecc<secp256k1> {
  static uint32_t get_private_key_size();
  static uint32_t get_public_key_size();
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

  static uint32_t get_encrypt_message_size_with_prefix(uint32_t data_size);
  static uint32_t encrypt_message_with_prefix(const uint8_t *public_key,
                                              uint32_t pkey_size,
                                              const uint8_t *data,
                                              uint32_t data_size,
                                              uint32_t prefix, uint8_t *cipher,
                                              uint32_t cipher_size);

  static uint32_t get_decrypt_message_size_with_prefix(uint32_t data_size);
  static uint32_t decrypt_message_with_prefix(const uint8_t *private_key,
                                              uint32_t private_key_size,
                                              const uint8_t *cipher,
                                              uint32_t cipher_size,
                                              uint8_t *data, uint32_t data_size,
                                              uint32_t prefix);

  static std::shared_ptr<internal::secp256k1_context_i> context;
};
} // namespace crypto
} // namespace stbox
