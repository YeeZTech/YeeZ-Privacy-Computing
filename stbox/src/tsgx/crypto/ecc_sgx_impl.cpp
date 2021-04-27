#include "common/crypto_prefix.h"
#include "ecc_t.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/crypto/ecc_context.h"

#define SECP256K1_PRIVATE_KEY_SIZE 32

uint64_t stbox_ecc_version() { return 1; }
uint32_t get_secp256k1_public_key_size() {
  return stbox::crypto::get_secp256k1_public_key_size();
}
uint32_t get_secp256k1_sealed_private_key_size() {
  return stbox::crypto::get_secp256k1_sealed_private_key_size();
}
uint32_t get_secp256k1_signature_size() {
  return stbox::crypto::get_secp256k1_signature_size();
}
uint32_t get_rijndael128GCM_encrypt_size(uint32_t data_size) {}
uint32_t get_rijndael128GCM_decrypt_size(uint32_t cipher_size) {}

uint32_t generate_secp256k1_key_pair(uint8_t *public_key, uint32_t pkey_size,
                                     uint8_t *sealed_private_key,
                                     uint32_t sealed_size) {
  if (public_key == NULL || sealed_private_key == NULL) {
    return SGX_ERROR_OUT_OF_MEMORY;
  }
  sgx_status_t se_ret;

  uint32_t skey_size = SECP256K1_PRIVATE_KEY_SIZE;
  uint8_t skey[SECP256K1_PRIVATE_KEY_SIZE];

  se_ret = (sgx_status_t)stbox::crypto::gen_secp256k1_skey(skey_size, skey);
  if (se_ret) {
    return se_ret;
  }

  se_ret = (sgx_status_t)stbox::crypto::generate_secp256k1_pkey_from_skey(
      skey, public_key, pkey_size);

  if (se_ret) {
    return se_ret;
  }

  return stbox::crypto::seal_secp256k1_private_key(skey, sealed_private_key,
                                                   sealed_size);
}

uint32_t generate_secp256k1_pkey_from_sealed_skey(uint8_t *sealed_key,
                                                  uint32_t sealed_size,
                                                  uint8_t *pkey,
                                                  uint32_t pkey_size) {
  uint8_t skey[SECP256K1_PRIVATE_KEY_SIZE];
  auto se_ret = stbox::crypto::unseal_secp256k1_private_key(sealed_key,
                                                            sealed_size, skey);
  if (se_ret) {
    return se_ret;
  }

  se_ret =
      stbox::crypto::generate_secp256k1_pkey_from_skey(skey, pkey, pkey_size);

  return se_ret;
}

uint32_t sign_message(uint8_t *sealed_private_key, uint32_t sealed_size,
                      uint8_t *data, uint32_t data_size, uint8_t *sig,
                      uint32_t sig_size) {

  uint8_t skey[SECP256K1_PRIVATE_KEY_SIZE];
  auto se_ret = stbox::crypto::unseal_secp256k1_private_key(sealed_private_key,
                                                            sealed_size, skey);
  if (se_ret) {
    return se_ret;
  }
  return stbox::crypto::sign_message(skey, SECP256K1_PRIVATE_KEY_SIZE, data,
                                     data_size, sig, sig_size);
}

uint32_t verify_signature(uint8_t *data, uint32_t data_size, uint8_t *sig,
                          uint32_t sig_size, uint8_t *public_key,
                          uint32_t pkey_size) {
  return stbox::crypto::verify_signature(data, data_size, sig, sig_size,
                                         public_key, pkey_size);
}

uint32_t get_encrypted_message_size(uint32_t data_size) {
  return stbox::crypto::get_encrypt_message_size_with_prefix(data_size);
}

uint32_t encrypt_message(uint8_t *public_key, uint32_t pkey_size, uint8_t *data,
                         uint32_t data_size, uint8_t *cipher,
                         uint32_t cipher_size) {
  return stbox::crypto::encrypt_message_with_prefix(
      public_key, pkey_size, data, data_size, ypc::crypto_prefix_arbitrary,
      cipher, cipher_size);
}

uint32_t get_decrypted_message_size(uint32_t cipher_size) {
  return stbox::crypto::get_decrypt_message_size_with_prefix(cipher_size);
}

uint32_t decrypt_message(uint8_t *sealed_private_key, uint32_t sealed_size,
                         uint8_t *cipher, uint32_t cipher_size, uint8_t *data,
                         uint32_t data_size) {

  uint8_t skey[SECP256K1_PRIVATE_KEY_SIZE];
  auto se_ret = stbox::crypto::unseal_secp256k1_private_key(sealed_private_key,
                                                            sealed_size, skey);
  if (se_ret) {
    return se_ret;
  }
  return stbox::crypto::decrypt_message_with_prefix(
      skey, SECP256K1_PRIVATE_KEY_SIZE, cipher, cipher_size, data, data_size,
      ypc::crypto_prefix_arbitrary);
}
