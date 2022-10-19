#include "ypc/corecommon/crypto/gmssl/sm4_aes.h"
#include "ypc/common/byte.h"
#include "ypc/corecommon/crypto/gmssl/sm3_hash.h"
#include "ypc/stbox/stx_status.h"
#include "ypc/stbox/tsgx/log.h"
#include <gmssl/sm4.h>
#include <sgx_tcrypto.h>
#include <sgx_trts.h>

#define AAD_MAC_TEXT_LEN 64
#define AAD_MAC_PREFIX_POS 24
static char aad_mac_text[AAD_MAC_TEXT_LEN] = "tech.yeez.key.manager";

namespace ypc {
namespace crypto {

uint32_t sm4_aes::encrypt_with_prefix(const uint8_t *key, uint32_t key_size,
                                      const uint8_t *data, uint32_t data_size,
                                      uint32_t prefix, uint8_t *cipher,
                                      uint32_t cipher_size, uint8_t *out_mac) {
  if (key_size != get_key_size()) {
    LOG(ERROR) << "invalid key size: " << key_size << ", expect 16!";
    return stbox::stx_status::ecc_invalid_aes_key_size;
  }

  if (cipher_size != data_size + INITIALIZATION_VECTOR_SIZE) {
    LOG(ERROR)
        << "cipher size should equal to data_size + INITIALIZATION_VECTOR_SIZE";
    return stbox::stx_status::aes_invalid_cipher_size;
  }

  uint8_t mac_text[AAD_MAC_TEXT_LEN] = {0};
  memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
  uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
  *p_prefix = prefix;
  uint8_t *p_iv_text = cipher + data_size;
  auto se_ret = sgx_read_rand(p_iv_text, INITIALIZATION_VECTOR_SIZE);
  if (se_ret) {
    LOG(ERROR) << "call sgx_read_rand failed";
    return se_ret;
  }

  SM4_KEY sm4_key;
  sm4_set_encrypt_key(&sm4_key, key);
  int ret =
      sm4_gcm_encrypt(&sm4_key, p_iv_text, INITIALIZATION_VECTOR_SIZE, mac_text,
                      AAD_MAC_TEXT_LEN, data, data_size, cipher, 16, out_mac);
  if (ret == -1) {
    return stbox::stx_status::sm4_encrypt_error;
  }
  return stbox::stx_status::success;
}

uint32_t sm4_aes::decrypt_with_prefix(const uint8_t *key, uint32_t key_size,
                                      const uint8_t *cipher,
                                      uint32_t cipher_size, uint32_t prefix,
                                      uint8_t *data, uint32_t data_size,
                                      const uint8_t *in_mac) {
  if (key_size != get_key_size()) {
    LOG(ERROR) << "invalid key size: " << key_size << ", expect 16!";
    return stbox::stx_status::ecc_invalid_aes_key_size;
  }

  if (cipher_size != data_size + INITIALIZATION_VECTOR_SIZE) {
    LOG(ERROR)
        << "cipher size should equal to data_size + INITIALIZATION_VECTOR_SIZE";
    return stbox::stx_status::aes_invalid_data_size;
  }
  uint8_t mac_text[AAD_MAC_TEXT_LEN] = {0};
  memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
  uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
  *p_prefix = prefix;
  const uint8_t *p_iv_text = cipher + data_size;

  SM4_KEY sm4_key;
  sm4_set_encrypt_key(&sm4_key, key);
  int ret = sm4_gcm_decrypt(&sm4_key, p_iv_text, INITIALIZATION_VECTOR_SIZE,
                            mac_text, AAD_MAC_TEXT_LEN, cipher, data_size,
                            in_mac, get_mac_code_size(), data);
  if (ret == -1) {
    return stbox::stx_status::sm4_decrypt_error;
  }
  return stbox::stx_status::success;
}

} // namespace crypto
} // namespace ypc
