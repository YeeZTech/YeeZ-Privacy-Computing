#include "stbox/tsgx/crypto/secp256k1/ecc_secp256k1.h"
#include "common/endian.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/scope_guard.h"
#include "stbox/stx_common.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/crypto/ecp_interface.h"
#include "stbox/tsgx/crypto/secp256k1/secp256k1_context_i.h"
#include "stbox/tsgx/log.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include "stbox/tsgx/secp256k1/secp256k1_preallocated.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"
#include <sgx_tcrypto.h>
#include <sgx_trts.h>
#include <sgx_tseal.h>
#include <string.h>

#define SECP256K1_PRIVATE_KEY_SIZE 32
#define INITIALIZATION_VECTOR_SIZE 12
#define SGX_AES_GCM_128BIT_TAG_T_SIZE sizeof(sgx_aes_gcm_128bit_tag_t)

using namespace stbox;

namespace stbox {
namespace crypto {

std::shared_ptr<internal::secp256k1_context_i>
    raw_ecc<secp256k1>::context(nullptr);

namespace internal {
//@pkey is little endian
uint32_t gen_sgx_ec_key_128bit(const uint8_t *pkey, uint32_t pkey_size,
                               const uint8_t *skey, uint32_t skey_size,
                               uint8_t *derived_key);
} // namespace internal
} // namespace crypto
} // namespace stbox

#define AAD_MAC_TEXT_LEN 64
#define AAD_MAC_PREFIX_POS 24
static char aad_mac_text[AAD_MAC_TEXT_LEN] = "tech.yeez.key.manager";
static uint8_t p_iv_text[INITIALIZATION_VECTOR_SIZE] = {
    89, 101, 101, 90, 70, 105, 100, 101, 108, 105, 117, 115}; //"YeeZFidelius";

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

uint32_t gen_pkey_from_skey(const secp256k1_context *ctx, const uint8_t *skey,
                            secp256k1_pubkey *pkey) {
  if (!ctx || !skey) {
    LOG(ERROR) << "Context or Secret key or Public key is null";
    return stbox::stx_status::ecc_invalid_ctx_or_skey;
  }
  auto ret = secp256k1_ec_pubkey_create(ctx, pkey, skey);
  if (!ret) {
    LOG(ERROR) << "Pubkey computation failed: " << ret;
    return stbox::stx_status::ecc_secp256k1_ec_pubkey_create_error;
  }
  ::ypc::utc::change_pubkey_endian((uint8_t *)pkey, sizeof(secp256k1_pubkey));
  return SGX_SUCCESS;
}

uint32_t sign_hash(const secp256k1_context *ctx, const uint8_t *hash,
                   const uint8_t *skey, uint8_t *sig) {
  secp256k1_ecdsa_recoverable_signature rsig;
  auto ret =
      secp256k1_ecdsa_sign_recoverable(ctx, &rsig, hash, skey, NULL, NULL);
  if (!ret) {
    LOG(ERROR) << "sign error: " << ret;
    return stbox::stx_status::ecc_secp256k1_ecdsa_sign_recoverable_error;
  }
  int recid;
  ret = secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, sig,
                                                                &recid, &rsig);
  if (!ret) {
    LOG(ERROR) << "serialize sig error: " << ret;
    return ecc_secp256k1_ecdsa_RSSC_error;
  }
  sig[64] = (uint8_t)(recid + 27);
  return SGX_SUCCESS;
}

namespace stbox {
namespace crypto {

secp256k1_context *init_secp256k1_context() {
  if (!raw_ecc<secp256k1>::context) {
    raw_ecc<secp256k1>::context =
        std::make_shared<internal::secp256k1_context_i>();
  }
  return raw_ecc<secp256k1>::context->ctx();
}

uint32_t raw_ecc<secp256k1>::get_private_key_size() { return 32; }

uint32_t raw_ecc<secp256k1>::get_public_key_size() {
  return sizeof(secp256k1_pubkey) * sizeof(unsigned char);
}
uint32_t raw_ecc<secp256k1>::gen_private_key(uint32_t skey_size,
                                             uint8_t *skey) {

  secp256k1_context *ctx = init_secp256k1_context();

  if (!ctx) {
    LOG(ERROR) << "Context or Secret key or Public key is null";
    return stbox::stx_status::ecc_invalid_ctx_or_skey;
  }
  sgx_status_t se_ret;
  do {
    se_ret = sgx_read_rand(skey, skey_size);
    if (se_ret != SGX_SUCCESS) {
      LOG(ERROR) << "call sgx_read_rand failed";
      return se_ret;
    }
  } while (!secp256k1_ec_seckey_verify(ctx, skey));
  return se_ret;
}

uint32_t raw_ecc<secp256k1>::generate_pkey_from_skey(const uint8_t *skey,
                                                     uint32_t skey_size,
                                                     uint8_t *pkey,
                                                     uint32_t pkey_size) {
  auto ctx = init_secp256k1_context();
  if (!ctx || !skey) {
    LOG(ERROR) << "Context or Secret key or Public key is null";
    return stbox::stx_status::ecc_invalid_ctx_or_skey;
  }
  auto ret = secp256k1_ec_pubkey_create(ctx, (secp256k1_pubkey *)pkey, skey);
  if (!ret) {
    LOG(ERROR) << "Pubkey computation failed: " << ret;
    return stbox::stx_status::ecc_secp256k1_ec_pubkey_create_error;
  }
  ::ypc::utc::change_pubkey_endian((uint8_t *)pkey, sizeof(secp256k1_pubkey));
  return SGX_SUCCESS;
}

uint32_t raw_ecc<secp256k1>::get_signature_size() {
  return 1 + sizeof(secp256k1_ecdsa_signature) * sizeof(unsigned char);
}
uint32_t raw_ecc<secp256k1>::sign_message(const uint8_t *skey,
                                          uint32_t skey_size,
                                          const uint8_t *data,
                                          uint32_t data_size, uint8_t *sig,
                                          uint32_t sig_size) {
  sgx_status_t se_ret;
  auto ctx = init_secp256k1_context();

  auto hash = stbox::eth::msg_hash(data, data_size);
  sig_size = get_signature_size();
  se_ret = (sgx_status_t)sign_hash(ctx, hash.data(), skey, sig);
  return se_ret;
}

uint32_t raw_ecc<secp256k1>::verify_signature(
    const uint8_t *data, uint32_t data_size, const uint8_t *sig,
    uint32_t sig_size, const uint8_t *public_key, uint32_t pkey_size) {
  sgx_status_t se_ret;
  auto ctx = init_secp256k1_context();

  secp256k1_pubkey secp256k1_pkey;
  memcpy(&secp256k1_pkey, public_key, pkey_size);
  ::ypc::utc::change_pubkey_endian((uint8_t *)&secp256k1_pkey,
                                   sizeof(secp256k1_pubkey));

  secp256k1_ecdsa_recoverable_signature rsig;
  se_ret = (sgx_status_t)secp256k1_ecdsa_recoverable_signature_parse_compact(
      ctx, &rsig, sig, *(sig + 64) - 27);
  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdsa_recoverable_signature_parse_compact return "
               << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256k1_ecdsa_sign_recoverable_error;
  }
  secp256k1_ecdsa_signature secp256k1_sig;
  se_ret = (sgx_status_t)secp256k1_ecdsa_recoverable_signature_convert(
      ctx, &secp256k1_sig, &rsig);
  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdsa_recoverable_signature_convert return "
               << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256k1_ecdsa_sign_recoverable_error;
  }

  auto hash = stbox::eth::msg_hash(data, data_size);
  se_ret = (sgx_status_t)secp256k1_ecdsa_verify(ctx, &secp256k1_sig,
                                                hash.data(), &secp256k1_pkey);
  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdsa_verify return " << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256_ecdsa_verify_error;
  }
  return SGX_SUCCESS;
}

uint32_t
raw_ecc<secp256k1>::get_encrypt_message_size_with_prefix(uint32_t data_size) {
  return data_size + get_public_key_size() + SGX_AES_GCM_128BIT_TAG_T_SIZE;
}
uint32_t
raw_ecc<secp256k1>::get_decrypt_message_size_with_prefix(uint32_t cipher_size) {
  return cipher_size - get_public_key_size() - SGX_AES_GCM_128BIT_TAG_T_SIZE;
}

uint32_t raw_ecc<secp256k1>::encrypt_message_with_prefix(
    const uint8_t *public_key, uint32_t pkey_size, const uint8_t *data,
    uint32_t data_size, uint32_t prefix, uint8_t *cipher,
    uint32_t cipher_size) {
  init_secp256k1_context();

  sgx_status_t se_ret;
  uint32_t _pkey_size = get_public_key_size();
  uint8_t *_public_key = cipher + data_size;
  const bytes &gskey = context->skey();
  const bytes &gpkey_big = context->pkey_big_endian();
  memcpy(_public_key, gpkey_big.data(), _pkey_size);

  sgx_ec_key_128bit_t derived_key;
  se_ret = (sgx_status_t)internal::gen_sgx_ec_key_128bit(
      public_key, pkey_size, gskey.data(), gskey.size(),
      (uint8_t *)&derived_key);
  if (se_ret) {
    return se_ret;
  }

  uint8_t mac_text[AAD_MAC_TEXT_LEN];
  memset(mac_text, 0, AAD_MAC_TEXT_LEN);
  memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
  uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
  *p_prefix = prefix;

  sgx_aes_gcm_128bit_tag_t *p_out_mac =
      (sgx_aes_gcm_128bit_tag_t *)(cipher + data_size + _pkey_size);
  se_ret = sgx_rijndael128GCM_encrypt(
      (const sgx_aes_gcm_128bit_key_t *)derived_key, data, data_size, cipher,
      p_iv_text, INITIALIZATION_VECTOR_SIZE, mac_text, AAD_MAC_TEXT_LEN,
      p_out_mac);
  return se_ret;
}

uint32_t raw_ecc<secp256k1>::decrypt_message_with_prefix(
    const uint8_t *skey, uint32_t skey_size, const uint8_t *cipher,
    uint32_t cipher_size, uint8_t *data, uint32_t data_size, uint32_t prefix) {
  init_secp256k1_context();

  sgx_status_t se_ret;
  sgx_ec_key_128bit_t derived_key;
  uint32_t pkey_size = get_public_key_size();
  se_ret = (sgx_status_t)internal::gen_sgx_ec_key_128bit(
      cipher + data_size, pkey_size, skey, skey_size, (uint8_t *)&derived_key);
  if (se_ret) {
    LOG(ERROR) << "gen_sgx_ec_key_128bit fail: " << se_ret;
    return se_ret;
  }

  uint8_t mac_text[AAD_MAC_TEXT_LEN];
  memset(mac_text, 0, AAD_MAC_TEXT_LEN);
  memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
  uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
  *p_prefix = prefix;

  se_ret = sgx_rijndael128GCM_decrypt(
      (const sgx_aes_gcm_128bit_key_t *)derived_key, cipher, data_size, data,
      p_iv_text, INITIALIZATION_VECTOR_SIZE, mac_text, AAD_MAC_TEXT_LEN,
      (const sgx_aes_gcm_128bit_tag_t *)(cipher + data_size + pkey_size));
  return se_ret;
}

namespace internal {

uint32_t gen_sgx_ec_key_128bit(const uint8_t *pkey, uint32_t pkey_size,
                               const uint8_t *skey, uint32_t skey_size,
                               uint8_t *derived_key) {
  sgx_status_t se_ret;

  secp256k1_pubkey lpkey;
  memcpy((uint8_t *)&lpkey, pkey, sizeof(secp256k1_pubkey));
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
                               (sgx_ec_key_128bit_t *)derived_key);
  return se_ret;
}
} // namespace internal

} // namespace crypto
} // namespace stbox

