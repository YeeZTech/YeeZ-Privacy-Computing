#include "common/crypto_prefix.h"
#include "corecommon/crypto/stdeth.h"
#include "ecc_t.h"
#include "stbox/tsgx/crypto/seal.h"
#include "stbox/tsgx/crypto/seal_sgx.h"
#include "stbox/tsgx/log.h"

using ecc = ypc::crypto::eth_sgx_crypto;
using raw_ecc = ecc;
using sealer = stbox::crypto::raw_device_sealer<stbox::crypto::intel_sgx>;

uint64_t stbox_ecc_version() { return 1; }
uint32_t get_secp256k1_public_key_size() { return ecc::get_public_key_size(); }
uint32_t get_secp256k1_sealed_private_key_size() {
  return sealer::get_sealed_data_size(ecc::get_private_key_size());
}

uint32_t get_secp256k1_signature_size() { return ecc::get_signature_size(); }

uint32_t generate_secp256k1_key_pair(uint8_t *public_key, uint32_t pkey_size,
                                     uint8_t *sealed_private_key,
                                     uint32_t sealed_size) {
  if (public_key == NULL || sealed_private_key == NULL) {
    return SGX_ERROR_OUT_OF_MEMORY;
  }
  sgx_status_t se_ret;

  stbox::bytes skey;

  se_ret = (sgx_status_t)ecc::gen_private_key(skey);
  if (se_ret) {
    LOG(ERROR) << "failed to gen private key " << se_ret;
    return se_ret;
  }

  se_ret = (sgx_status_t)raw_ecc::generate_pkey_from_skey(
      skey.data(), skey.size(), public_key, pkey_size);

  if (se_ret) {
    LOG(ERROR) << "failed to gen public key " << se_ret;
    return se_ret;
  }
  return sealer::seal_data(skey.data(), skey.size(), sealed_private_key,
                           sealed_size);
}

uint32_t generate_secp256k1_pkey_from_sealed_skey(uint8_t *sealed_key,
                                                  uint32_t sealed_size,
                                                  uint8_t *pkey,
                                                  uint32_t pkey_size) {
  stbox::bytes skey(ecc::get_private_key_size());
  auto se_ret =
      sealer::unseal_data(sealed_key, sealed_size, skey.data(), skey.size());
  if (se_ret) {
    LOG(ERROR) << "failed to unseal private key " << se_ret;
    return se_ret;
  }

  se_ret = raw_ecc::generate_pkey_from_skey(skey.data(), skey.size(), pkey,
                                            pkey_size);

  return se_ret;
}

uint32_t sign_message(uint8_t *sealed_private_key, uint32_t sealed_size,
                      uint8_t *data, uint32_t data_size, uint8_t *sig,
                      uint32_t sig_size) {
  stbox::bytes skey(ecc::get_private_key_size());
  auto se_ret = sealer::unseal_data(sealed_private_key, sealed_size,
                                    skey.data(), skey.size());
  if (se_ret) {
    LOG(ERROR) << "failed to unseal private key " << se_ret;
    return se_ret;
  }
  return raw_ecc::sign_message(skey.data(), skey.size(), data, data_size, sig,
                               sig_size);
}

uint32_t verify_signature(uint8_t *data, uint32_t data_size, uint8_t *sig,
                          uint32_t sig_size, uint8_t *public_key,
                          uint32_t pkey_size) {
  return raw_ecc::verify_signature(data, data_size, sig, sig_size, public_key,
                                   pkey_size);
}

uint32_t get_encrypted_message_size(uint32_t data_size) {
  return raw_ecc::get_encrypt_message_size_with_prefix(data_size);
}

uint32_t encrypt_message(uint8_t *public_key, uint32_t pkey_size, uint8_t *data,
                         uint32_t data_size, uint8_t *cipher,
                         uint32_t cipher_size) {
  return raw_ecc::encrypt_message_with_prefix(
      public_key, pkey_size, data, data_size, ypc::utc::crypto_prefix_arbitrary,
      cipher, cipher_size);
}

uint32_t get_decrypted_message_size(uint32_t cipher_size) {
  return raw_ecc::get_decrypt_message_size_with_prefix(cipher_size);
}

uint32_t decrypt_message(uint8_t *sealed_private_key, uint32_t sealed_size,
                         uint8_t *cipher, uint32_t cipher_size, uint8_t *data,
                         uint32_t data_size) {
  stbox::bytes skey(ecc::get_private_key_size());
  auto se_ret = sealer::unseal_data(sealed_private_key, sealed_size,
                                    skey.data(), skey.size());
  if (se_ret) {
    LOG(ERROR) << "failed to unseal private key " << se_ret;
    return se_ret;
  }

  se_ret = raw_ecc::decrypt_message_with_prefix(
      skey.data(), skey.size(), cipher, cipher_size, data, data_size,
      ypc::utc::crypto_prefix_arbitrary);
  return se_ret;
}
