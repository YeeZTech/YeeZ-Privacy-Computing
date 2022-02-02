#include "corecommon/crypto/stdeth/secp256k1_ecdh_sgx128.h"
#include "common/endian.h"
#include "stbox/tsgx/log.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include <sgx_ecp_types.h>
#include <sgx_tcrypto.h>
#include <sgx_trts.h>

#define AAD_MAC_TEXT_LEN 64
static char aad_mac_text[AAD_MAC_TEXT_LEN] = "tech.yeez.key.manager";

#define MAC_KEY_SIZE 16
static uint8_t cmac_key[MAC_KEY_SIZE] = "yeez.tech.stbox";
#define EC_DERIVATION_BUFFER_SIZE(label_length) ((label_length) + 4)

#ifndef INTERNAL_SGX_ERROR_CODE_CONVERTOR
#define INTERNAL_SGX_ERROR_CODE_CONVERTOR(x)                                   \
  if (x != SGX_ERROR_OUT_OF_MEMORY) {                                          \
    x = SGX_ERROR_UNEXPECTED;                                                  \
  }
#endif

namespace ypc {
namespace crypto {
extern secp256k1_context *init_secp256k1_context();

static int ecdh_hash_function_sha256(unsigned char *output,
                                     const unsigned char *x,
                                     const unsigned char *y, void *data) {
  uint8_t buf[33];
  unsigned char version = (y[31] & 0x01) | 0x02;
  buf[0] = version;
  memcpy(buf + 1, x, 32);
  sgx_sha256_msg(buf, 33, (sgx_sha256_hash_t *)output);
  return 1;
}

static sgx_status_t derive_key(const sgx_ec256_dh_shared_t *shared_key,
                               const char *label, uint32_t label_length,
                               sgx_ec_key_128bit_t *derived_key) {
  sgx_status_t se_ret = SGX_SUCCESS;
  sgx_ec_key_128bit_t key_derive_key;
  if (!shared_key || !derived_key || !label) {
    return SGX_ERROR_INVALID_PARAMETER;
  }

  /*check integer overflow */
  if (label_length > EC_DERIVATION_BUFFER_SIZE(label_length)) {
    return SGX_ERROR_INVALID_PARAMETER;
  }

  se_ret = sgx_rijndael128_cmac_msg(
      (sgx_cmac_128bit_key_t *)cmac_key, (uint8_t *)shared_key,
      sizeof(sgx_ec256_dh_shared_t), (sgx_cmac_128bit_tag_t *)&key_derive_key);
  if (SGX_SUCCESS != se_ret) {
    memset_s(&key_derive_key, sizeof(key_derive_key), 0,
             sizeof(key_derive_key));
    INTERNAL_SGX_ERROR_CODE_CONVERTOR(se_ret);
    return se_ret;
  }

  // TODO: note this is quite common, we may optimize this into 1 computation
  /* derivation_buffer = counter(0x01) || label || 0x00 ||
   * output_key_len(0x0080) */
  uint32_t derivation_buffer_length = EC_DERIVATION_BUFFER_SIZE(label_length);
  uint8_t *p_derivation_buffer = (uint8_t *)malloc(derivation_buffer_length);
  if (p_derivation_buffer == NULL) {
    return SGX_ERROR_OUT_OF_MEMORY;
  }
  memset(p_derivation_buffer, 0, derivation_buffer_length);

  /*counter = 0x01 */
  p_derivation_buffer[0] = 0x01;
  /*label*/
  memcpy(&p_derivation_buffer[1], label, label_length);
  /*output_key_len=0x0080*/
  uint16_t *key_len =
      (uint16_t *)&p_derivation_buffer[derivation_buffer_length - 2];
  *key_len = 0x0080;

  se_ret = sgx_rijndael128_cmac_msg(
      (sgx_cmac_128bit_key_t *)&key_derive_key, p_derivation_buffer,
      derivation_buffer_length, (sgx_cmac_128bit_tag_t *)derived_key);
  memset_s(&key_derive_key, sizeof(key_derive_key), 0, sizeof(key_derive_key));
  free(p_derivation_buffer);
  if (SGX_SUCCESS != se_ret) {
    INTERNAL_SGX_ERROR_CODE_CONVERTOR(se_ret);
  }
  return se_ret;
}

uint32_t secp256k1_ecdh_sgx128::ecdh_shared_key(
    const uint8_t *skey, uint32_t skey_size, const uint8_t *public_key,
    uint32_t pkey_size, uint8_t *shared_key, uint32_t shared_key_size) {
  if (shared_key_size != 16) {
    LOG(ERROR) << "invalid aes key size " << shared_key_size << ", expect 16";
    return stbox::stx_status::ecc_invalid_aes_key_size;
  }
  if (skey_size != 32) {
    LOG(ERROR) << "invalid skey size " << skey_size << ", expect 32";
    return stbox::stx_status::ecc_invalid_skey_size;
  }
  if (pkey_size != 64) {
    LOG(ERROR) << "invalid pkey size " << pkey_size << ", expect 64";
    return stbox::stx_status::ecc_invalid_pkey_size;
  }

  sgx_status_t se_ret;

  secp256k1_pubkey lpkey;
  memcpy((uint8_t *)&lpkey, public_key, sizeof(secp256k1_pubkey));
  ::ypc::utc::change_pubkey_endian((uint8_t *)&lpkey, sizeof(secp256k1_pubkey));

  auto ctx = init_secp256k1_context();
  sgx_ec256_dh_shared_t ec256_dh_shared_key;
  se_ret =
      (sgx_status_t)secp256k1_ecdh(ctx, (uint8_t *)&ec256_dh_shared_key, &lpkey,
                                   skey, ecdh_hash_function_sha256, NULL);

  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdh returns: " << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256k1_ecdh_error;
  }
  uint32_t aad_mac_len = strlen(aad_mac_text);
  se_ret =
      (sgx_status_t)derive_key(&ec256_dh_shared_key, aad_mac_text, aad_mac_len,
                               (sgx_ec_key_128bit_t *)shared_key);
  return se_ret;
}
}
} // namespace ypc
