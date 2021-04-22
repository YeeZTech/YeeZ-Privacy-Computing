#pragma once
#include <cstdint>
#include <memory>

namespace stbox {
namespace crypto {

// In this namespace, the pkey is big endian by default.

uint32_t get_secp256k1_public_key_size();

uint32_t gen_secp256k1_skey(uint32_t skey_size, uint8_t *skey);

uint32_t get_secp256k1_sealed_private_key_size();

uint32_t generate_secp256k1_pkey_from_skey(const uint8_t *skey, uint8_t *pkey,
                                           uint32_t pkey_size);


// this is eth sig compatiable
uint32_t get_secp256k1_signature_size();
uint32_t sign_message(const uint8_t *sealed_private_key, uint32_t sealed_size,
                      const uint8_t *data, uint32_t data_size, uint8_t *sig,
                      uint32_t sig_size);
uint32_t verify_signature(const uint8_t *data, uint32_t data_size,
                          const uint8_t *sig, uint32_t sig_size,
                          const uint8_t *public_key, uint32_t pkey_size);

uint32_t get_encrypt_message_size_with_prefix(uint32_t data_size);
uint32_t encrypt_message_with_prefix(const uint8_t *public_key,
                                     uint32_t pkey_size, const uint8_t *data,
                                     uint32_t data_size, uint32_t prefix,
                                     uint8_t *cipher, uint32_t cipher_size);

uint32_t get_decrypt_message_size_with_prefix(uint32_t data_size);
uint32_t decrypt_message_with_prefix(const uint8_t *private_key,
                                     uint32_t private_key_size,
                                     const uint8_t *cipher,
                                     uint32_t cipher_size, uint8_t *data,
                                     uint32_t data_size, uint32_t prefix);

uint32_t seal_secp256k1_private_key(const uint8_t *skey,
                                    uint8_t *sealed_private_key,
                                    uint32_t sealed_size);

uint32_t unseal_secp256k1_private_key(const uint8_t *sealed_private_key,
                                      uint32_t sealed_size, uint8_t *skey);

// We put the following functions into internal in order to remind users that
// the should not directly use them.
namespace internal {
uint32_t get_rijndael128GCM_encrypt_size(uint32_t data_size);

uint32_t get_rijndael128GCM_decrypt_size(uint32_t cipher_size);

//@pkey is little endian
uint32_t gen_sgx_ec_key_128bit(const uint8_t *pkey, uint32_t pkey_size,
                               const uint8_t *skey, uint32_t skey_size,
                               uint8_t *derived_key);
} // namespace internal

class ecc_context;
extern std::shared_ptr<ecc_context> context;
} // namespace crypto
} // namespace stbox
