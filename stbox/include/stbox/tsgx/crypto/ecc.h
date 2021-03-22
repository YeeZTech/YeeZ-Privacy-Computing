#pragma once
#include <cstdint>

// TODO: we should involve better C++ style interface here
namespace stbox {
namespace crypto {
uint32_t get_secp256k1_public_key_size();
uint32_t get_secp256k1_sealed_private_key_size();
uint32_t generate_secp256k1_key_pair(uint8_t *pub_key, uint32_t pkey_size,
                                     uint8_t *sealed_priv_key,
                                     uint32_t sealed_size);
uint32_t generate_secp256k1_pkey_from_skey(const uint8_t *skey,
                                           uint32_t skey_size, uint8_t *pkey,
                                           uint32_t pkey_size);

uint32_t get_secp256k1_signature_size();
uint32_t sign_message(const uint8_t *sealed_private_key, uint32_t sealed_size,
                      const uint8_t *data, uint32_t data_size, uint8_t *sig,
                      uint32_t sig_size);
uint32_t verify_signature(const uint8_t *data, uint32_t data_size,
                          const uint8_t *sig, uint32_t sig_size,
                          const uint8_t *public_key, uint32_t pkey_size);

uint32_t get_rijndael128GCM_encrypt_size(uint32_t data_size);
uint32_t encrypt_message(const uint8_t *public_key, uint32_t pkey_size,
                         const uint8_t *data, uint32_t data_size,
                         uint8_t *cipher, uint32_t cipher_size);

uint32_t get_rijndael128GCM_decrypt_size(uint32_t cipher_size);
uint32_t decrypt_message(const uint8_t *sealed_private_key,
                         uint32_t sealed_size, const uint8_t *cipher,
                         uint32_t cipher_size, uint8_t *data,
                         uint32_t data_size);

} // namespace crypto
} // namespace stbox
