#include "ypc/corecommon/crypto/gmssl/sm2_ecc.h"
#include "ypc/common/byte.h"
#include "ypc/corecommon/crypto/gmssl/sm3_hash.h"
#include "ypc/stbox/gmssl/sm2.h"
#include "ypc/stbox/stx_status.h"
#include "ypc/stbox/tsgx/log.h"

namespace ypc {
namespace crypto {

uint32_t sm2_ecc::gen_private_key(uint32_t skey_size, uint8_t *skey) {
  SM2_KEY tmp;
  int res = sm2_key_generate(&tmp);
  if (res == -1) {
    return stbox::stx_status::sm2_empty_key;
  }
  memcpy(skey, tmp.private_key, skey_size);
  return stbox::stx_status::success;
}

uint32_t sm2_ecc::generate_pkey_from_skey(const uint8_t *skey,
                                          uint32_t skey_size, uint8_t *pkey,
                                          uint32_t pkey_size) {
  SM2_KEY tmp;
  int res = sm2_key_set_private_key(&tmp, skey);
  if (res == -1) {
    return stbox::stx_status::sm2_point_generate_error;
  }
  memcpy(pkey, &tmp, pkey_size);
  return stbox::stx_status::success;
}

uint32_t sm2_ecc::sign_message(const uint8_t *skey, uint32_t skey_size,
                               const uint8_t *data, uint32_t data_size,
                               uint8_t *sig, uint32_t sig_size) {
  SM2_KEY key;
  int res = sm2_key_set_private_key(&key, skey);
  if (res == -1) {
    return stbox::stx_status::sm2_point_generate_error;
  }

  SM2_SIGNATURE sm2_sig;
  int sign_res = sm2_do_sign(&key, data, &sm2_sig);
  if (sign_res == -1) {
    return stbox::stx_status::sm2_sign_error;
  }
  memcpy(sig, &sm2_sig, sig_size);

  return stbox::stx_status::success;
}

uint32_t sm2_ecc::verify_signature(const uint8_t *data, uint32_t data_size,
                                   const uint8_t *sig, uint32_t sig_size,
                                   const uint8_t *public_key,
                                   uint32_t pkey_size) {
  SM2_KEY key;
  SM2_POINT sm2_public_key;
  memcpy(&sm2_public_key, public_key, pkey_size);
  int res = sm2_key_set_public_key(&key, &sm2_public_key);
  if (res == -1) {
    return stbox::stx_status::sm2_empty_public_error;
  }

  SM2_SIGNATURE sm2_sig;
  memcpy(&sm2_sig, sig, sig_size);
  int verify_res = sm2_do_verify(&key, data, &sm2_sig);
  if (verify_res == -1) {
    return stbox::stx_status::sm2_verify_error;
  }
  if (verify_res == 0) {
    return stbox::stx_status::sm2_get_false_sign;
  }
  return stbox::stx_status::success;
}

uint32_t sm2_ecc::ecdh_shared_key(const uint8_t *skey, uint32_t skey_size,
                                  const uint8_t *public_key, uint32_t pkey_size,
                                  uint8_t *shared_key,
                                  uint32_t shared_key_size) {
  if (shared_key_size != sm2_ecc::get_ecdh_shared_key_size()) {
    LOG(ERROR) << "invalid aes key size " << shared_key_size << ", expect "
               << sm2_ecc::get_ecdh_shared_key_size();
    return stbox::stx_status::ecc_invalid_aes_key_size;
  }
  if (skey_size != sm2_ecc::get_private_key_size()) {
    LOG(ERROR) << "invalid skey size " << skey_size << ", expect "
               << sm2_ecc::get_private_key_size();
    return stbox::stx_status::ecc_invalid_skey_size;
  }
  if (pkey_size != sm2_ecc::get_public_key_size()) {
    LOG(ERROR) << "invalid pkey size " << pkey_size << ", expect "
               << sm2_ecc::get_public_key_size();
    return stbox::stx_status::ecc_invalid_pkey_size;
  }

  SM2_KEY key;
  int res = sm2_key_set_private_key(&key, skey);
  if (res == -1) {
    return stbox::stx_status::sm2_point_generate_error;
  }

  SM2_POINT peer_public, out;
  memcpy(&peer_public, public_key, pkey_size);

  int ecdh_res = sm2_ecdh(&key, &peer_public, &out);
  if (ecdh_res == -1) {
    return stbox::stx_status::sm2_shared_key_error;
  }

  uint32_t hash_size = sm3_hash::get_msg_hash_size();
  uint8_t tmp_shared_key[pkey_size], hash[hash_size];
  memcpy(tmp_shared_key, &out, pkey_size);
  sm3_hash::msg_hash(tmp_shared_key, pkey_size, hash, hash_size);
  memcpy(shared_key, hash, shared_key_size);
  return stbox::stx_status::success;
}

} // namespace crypto
} // namespace ypc

