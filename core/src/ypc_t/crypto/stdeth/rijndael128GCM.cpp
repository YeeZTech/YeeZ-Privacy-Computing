#include "corecommon/crypto/stdeth/rijndael128GCM.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/log.h"
#include <sgx_tcrypto.h>
#include <sgx_trts.h>

#define AAD_MAC_TEXT_LEN 64
#define AAD_MAC_PREFIX_POS 24
#define INITIALIZATION_VECTOR_SIZE 12
static char aad_mac_text[AAD_MAC_TEXT_LEN] = "tech.yeez.key.manager";
// static uint8_t p_iv_text[INITIALIZATION_VECTOR_SIZE] = {
// 89, 101, 101, 90, 70, 105, 100, 101, 108, 105, 117, 115}; //"YeeZFidelius";

namespace ypc {
namespace crypto {

uint32_t rijndael128GCM::get_cipher_size(uint32_t data_size) {
  return data_size + INITIALIZATION_VECTOR_SIZE;
}
uint32_t rijndael128GCM::get_data_size(uint32_t cipher_size) {
  if (cipher_size < INITIALIZATION_VECTOR_SIZE) {
    LOG(ERROR) << "invalid cipher size, should be bigger than 12";
    throw std::runtime_error("invalid cipher size");
  }
  return cipher_size - INITIALIZATION_VECTOR_SIZE;
}
uint32_t
rijndael128GCM::encrypt_with_prefix(const uint8_t *key, uint32_t key_size,
                                    const uint8_t *data, uint32_t data_size,
                                    uint32_t prefix, uint8_t *cipher,
                                    uint32_t cipher_size, uint8_t *out_mac) {
  if (key_size != 16) {
    LOG(ERROR) << "invalid key size: " << key_size << ", expect 16!";
    return stbox::stx_status::ecc_invalid_aes_key_size;
  }

  if (cipher_size != data_size + INITIALIZATION_VECTOR_SIZE) {
    LOG(ERROR) << "cipher size (" << cipher_size
               << ") should equal to data_size (" << data_size
               << ") + INITIALIZATION_VECTOR_SIZE";
    return stbox::stx_status::aes_invalid_cipher_size;
  }

  uint8_t mac_text[AAD_MAC_TEXT_LEN];
  memset(mac_text, 0, AAD_MAC_TEXT_LEN);
  memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
  uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
  *p_prefix = prefix;

  uint8_t *p_iv_text = cipher + data_size;
  auto se_ret = sgx_read_rand(p_iv_text, INITIALIZATION_VECTOR_SIZE);
  if (se_ret) {
    LOG(ERROR) << "call sgx_read_rand failed";
    return se_ret;
  }
  sgx_aes_gcm_128bit_tag_t *p_out_mac = (sgx_aes_gcm_128bit_tag_t *)out_mac;
  se_ret = sgx_rijndael128GCM_encrypt(
      (const sgx_aes_gcm_128bit_key_t *)key, data, data_size, cipher, p_iv_text,
      INITIALIZATION_VECTOR_SIZE, mac_text, AAD_MAC_TEXT_LEN, p_out_mac);
  return se_ret;
}

uint32_t
rijndael128GCM::decrypt_with_prefix(const uint8_t *key, uint32_t key_size,
                                    const uint8_t *cipher, uint32_t cipher_size,
                                    uint32_t prefix, uint8_t *data,
                                    uint32_t data_size, const uint8_t *in_mac) {
  if (key_size != 16) {
    LOG(ERROR) << "invalid key size: " << key_size << ", expect 16!";
    return stbox::stx_status::ecc_invalid_aes_key_size;
  }

  if (cipher_size != data_size + INITIALIZATION_VECTOR_SIZE) {
    LOG(ERROR) << "cipher size (" << cipher_size
               << ") should equal to data_size (" << data_size
               << ") + INITIALIZATION_VECTOR_SIZE";
    return stbox::stx_status::aes_invalid_data_size;
  }

  uint8_t mac_text[AAD_MAC_TEXT_LEN];
  memset(mac_text, 0, AAD_MAC_TEXT_LEN);
  memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
  uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
  *p_prefix = prefix;

  const uint8_t *p_iv_text = cipher + data_size;

  auto se_ret = sgx_rijndael128GCM_decrypt(
      (const sgx_aes_gcm_128bit_key_t *)key, cipher, data_size, data, p_iv_text,
      INITIALIZATION_VECTOR_SIZE, mac_text, AAD_MAC_TEXT_LEN,
      (const sgx_aes_gcm_128bit_tag_t *)in_mac);
  return se_ret;
}

} // namespace crypto
} // namespace ypc
