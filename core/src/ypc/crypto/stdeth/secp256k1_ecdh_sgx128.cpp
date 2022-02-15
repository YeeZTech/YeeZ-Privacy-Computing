#include "corecommon/crypto/stdeth/secp256k1_ecdh_sgx128.h"
#include "./openssl.h"
#include "common/endian.h"
#include "stbox/stx_status.h"
#include <secp256k1.h>
#include <secp256k1_ecdh.h>

#define AAD_MAC_TEXT_LEN 64
static char aad_mac_text[AAD_MAC_TEXT_LEN] = "tech.yeez.key.manager";

#define MAC_KEY_SIZE 16
static uint8_t cmac_key[MAC_KEY_SIZE] = "yeez.tech.stbox";
#define EC_DERIVATION_BUFFER_SIZE(label_length) ((label_length) + 4)

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
  ::ypc::openssl::sgx::sha256_msg(buf, 33, output);
  return 1;
}

int derive_key(const uint8_t *shared_key, size_t shared_key_len,
               const char *label, uint32_t label_length,
               uint8_t *derived_128bit_key) {
  int se_ret = 0;
  ypc::bytes key_derive_key(16);

  // memset(cmac_key, 0, MAC_KEY_SIZE);

  se_ret = ::ypc::openssl::sgx::rijndael128_cmac_msg(
      cmac_key, shared_key, shared_key_len, key_derive_key.data());
  if (se_ret) {
    return se_ret;
  }

  uint32_t derivation_buffer_length = EC_DERIVATION_BUFFER_SIZE(label_length);
  ypc::bytes p_derivation_buffer(derivation_buffer_length);
  memset(p_derivation_buffer.data(), p_derivation_buffer.size(), 0);
  /*counter = 0x01 */
  p_derivation_buffer[0] = 0x01;
  /*label*/
  memcpy(&p_derivation_buffer[1], label, label_length);
  /*output_key_len=0x0080*/
  p_derivation_buffer[p_derivation_buffer.size() - 3] = 0;
  uint16_t *key_len =
      (uint16_t *)&p_derivation_buffer[derivation_buffer_length - 2];
  *key_len = 0x0080;

  se_ret = ::ypc::openssl::sgx::rijndael128_cmac_msg(
      key_derive_key.data(), p_derivation_buffer.data(),
      derivation_buffer_length, derived_128bit_key);
  if (se_ret) {
    return se_ret;
  }
  return 0;
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


  secp256k1_pubkey lpkey;
  memcpy((uint8_t *)&lpkey, public_key, sizeof(secp256k1_pubkey));
  ::ypc::utc::change_pubkey_endian((uint8_t *)&lpkey, sizeof(secp256k1_pubkey));

  auto ctx = init_secp256k1_context();
  bytes ec256_dh_shared_key(32);
  auto se_ret = secp256k1_ecdh(ctx, ec256_dh_shared_key.data(), &lpkey, skey,
                               ecdh_hash_function_sha256, NULL);

  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdh returns: " << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256k1_ecdh_error;
  }
  uint32_t aad_mac_len = strlen(aad_mac_text);
  se_ret = derive_key(ec256_dh_shared_key.data(), ec256_dh_shared_key.size(),
                      aad_mac_text, aad_mac_len, shared_key);
  return se_ret;
}
}
} // namespace ypc
