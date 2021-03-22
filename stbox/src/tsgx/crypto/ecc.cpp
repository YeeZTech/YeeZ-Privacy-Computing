#include "ecc_t.h"
#include "stbox/scope_guard.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/crypto/ecp_interface.h"
#include <sgx_tcrypto.h>
#include <sgx_trts.h>
#include <sgx_tseal.h>
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <stdlib.h>
#include <string.h>

#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include "stbox/tsgx/secp256k1/secp256k1_preallocated.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"

#define SECP256K1_PRIVATE_KEY_SIZE 32
#define INITIALIZATION_VECTOR_SIZE 12
#define SGX_AES_GCM_128BIT_TAG_T_SIZE sizeof(sgx_aes_gcm_128bit_tag_t)

uint64_t stbox_ecc_version() { return 1; }
using namespace stbox;

// !! p_additional_MACtext_length[in, out]
static char aad_mac_text[BUFSIZ] = "tech.yeez.key.manager";
const static uint8_t *p_iv_text =
    (uint8_t *)malloc(INITIALIZATION_VECTOR_SIZE * sizeof(uint8_t));

uint32_t get_secp256k1_public_key_size() {
  return sizeof(secp256k1_pubkey) * sizeof(unsigned char);
}
uint32_t get_secp256k1_sealed_private_key_size() {
  return sgx_calc_sealed_data_size(
      strlen(aad_mac_text), SECP256K1_PRIVATE_KEY_SIZE * sizeof(unsigned char));
}
uint32_t get_secp256k1_signature_size() {
  return 1 + sizeof(secp256k1_ecdsa_signature) * sizeof(unsigned char);
}
uint32_t get_rijndael128GCM_encrypt_size(uint32_t data_size) {
  return data_size + get_secp256k1_public_key_size() +
         get_secp256k1_signature_size() + SGX_AES_GCM_128BIT_TAG_T_SIZE;
}
uint32_t get_rijndael128GCM_decrypt_size(uint32_t cipher_size) {
  return cipher_size - get_secp256k1_public_key_size() -
         get_secp256k1_signature_size() - SGX_AES_GCM_128BIT_TAG_T_SIZE;
}

uint32_t gen_skey(const secp256k1_context *ctx, uint32_t skey_size,
                  uint8_t *skey) {
  if (!ctx) {
    printf("Context or Secret key is null\n");
    return SGX_ERROR_UNEXPECTED;
  }
  sgx_status_t se_ret;
  do {
    se_ret = sgx_read_rand(skey, skey_size);
    if (se_ret != SGX_SUCCESS) {
      printf("Failed to generate rand skey\n");
      return SGX_ERROR_UNEXPECTED;
    }
  } while (!secp256k1_ec_seckey_verify(ctx, skey));
  return se_ret;
}

uint32_t gen_pkey_from_skey(const secp256k1_context *ctx, const uint8_t *skey,
                            secp256k1_pubkey *pkey) {
  if (!ctx || !skey) {
    // TODO: return error code
    printf("Context or Secret key or Public key is null\n");
    return SGX_ERROR_UNEXPECTED;
  }
  if (!secp256k1_ec_pubkey_create(ctx, pkey, skey)) {
    printf("Pubkey computation failed\n");
    return SGX_ERROR_UNEXPECTED;
  }
  return SGX_SUCCESS;
}

uint32_t sign_hash(const secp256k1_context *ctx, const uint8_t *hash,
                   const uint8_t *skey, uint8_t *sig) {
  secp256k1_ecdsa_recoverable_signature rsig;
  if (!secp256k1_ecdsa_sign_recoverable(ctx, &rsig, hash, skey, NULL, NULL)) {
    printf("Can't sign\n");
    return SGX_ERROR_UNEXPECTED;
  }
  int recid;
  if (!secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, sig, &recid,
                                                               &rsig)) {
    printf("Can't serialize the signature\n");
    return SGX_ERROR_UNEXPECTED;
  }
  sig[64] = (uint8_t)(recid + 27);
  return SGX_SUCCESS;
}

uint32_t generate_secp256k1_key_pair(uint8_t *public_key, uint32_t pkey_size,
                                     uint8_t *sealed_private_key,
                                     uint32_t sealed_size) {
#ifdef STBOX_CRYPTO_VERBOSE
  printf("\n########## generate secp256k1 key pair ##########\n");
#endif
  if (public_key == NULL || sealed_private_key == NULL) {
    return SGX_ERROR_OUT_OF_MEMORY;
  }
  sgx_status_t se_ret;

  uint32_t skey_size = SECP256K1_PRIVATE_KEY_SIZE;
  uint8_t *skey = (uint8_t *)malloc(skey_size * sizeof(uint8_t));
  malloc_memory_guard<uint8_t> _sl(skey);

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  se_ret = (sgx_status_t)gen_skey(ctx, skey_size, skey);
  if (se_ret) {
    return se_ret;
  }
#ifdef STBOX_CRYPTO_VERBOSE
  printf("secret key size: %u\n", skey_size);
  printf("secret key hex: ");
  print_hex(skey, skey_size);
#endif
  secp256k1_pubkey pub_key;
  se_ret = (sgx_status_t)gen_pkey_from_skey(ctx, skey, &pub_key);
  if (se_ret) {
    return se_ret;
  }

#ifdef STBOX_CRYPTO_VERBOSE
  printf("public key size: %d\n", pkey_size);
  printf("public key hex: ");
  print_hex(pub_key.data, pkey_size);
#endif
  memcpy(public_key, &pub_key, sizeof(pub_key));
#ifdef STBOX_CRYPTO_VERBOSE
  printf("copied public key hex: ");
  print_hex(public_key, pkey_size);
  // seal private key
  printf("sealed secret key size: %d\n", sealed_size);
#endif
  se_ret = sgx_seal_data(
      (const uint32_t)strlen(aad_mac_text), (const uint8_t *)aad_mac_text,
      (const uint32_t)skey_size, (const uint8_t *)skey,
      (const uint32_t)sealed_size, (sgx_sealed_data_t *)sealed_private_key);
#ifdef STBOX_CRYPTO_VERBOSE
  printf("sealed secret key hex: ");
  print_hex(sealed_private_key, sealed_size);
#endif

  secp256k1_context_destroy(ctx);
  return se_ret;
}

uint32_t generate_secp256k1_pkey_from_skey(const uint8_t *skey,
                                           uint32_t skey_size, uint8_t *pkey,
                                           uint32_t pkey_size) {
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  secp256k1_pubkey pub_key;
  auto se_ret = (sgx_status_t)gen_pkey_from_skey(ctx, skey, &pub_key);
  if (se_ret) {
    return se_ret;
  }
  memcpy(pkey, &pub_key, sizeof(pub_key));
  return se_ret;
}

uint32_t sign_message(uint8_t *sealed_private_key, uint32_t sealed_size,
                      uint8_t *data, uint32_t data_size, uint8_t *sig,
                      uint32_t sig_size) {
#ifdef STBOX_CRYPTO_VERBOSE
  printf("\n########## sign message ##########\n");
  printf("sealed secret key hex: ");
  print_hex(sealed_private_key, sealed_size);
#endif
  sgx_status_t se_ret;
  uint32_t aad_mac_len = strlen(aad_mac_text);
  uint32_t skey_size = sealed_size;
  if (SECP256K1_PRIVATE_KEY_SIZE != sealed_size) {
    skey_size =
        sgx_get_encrypt_txt_len((sgx_sealed_data_t *)sealed_private_key);
  }
#ifdef STBOX_CRYPTO_VERBOSE
  printf("get encrypt txt len: %u\n", skey_size);
#endif
  uint8_t *skey;
  ff::scope_guard _skey_desc([&]() { skey = new uint8_t[skey_size]; },
                             [&]() { delete[] skey; });
  if (SECP256K1_PRIVATE_KEY_SIZE != sealed_size) {
    se_ret = sgx_unseal_data((const sgx_sealed_data_t *)sealed_private_key,
                             (uint8_t *)aad_mac_text, (uint32_t *)&aad_mac_len,
                             skey, &skey_size);
    if (se_ret) {
      return se_ret;
    }
  } else {
    memcpy((uint8_t *)skey, sealed_private_key, sealed_size);
  }
#ifdef STBOX_CRYPTO_VERBOSE
  printf("unseal secret size: %u\n", skey_size);
  printf("unseal secret key hex: ");
  print_hex(skey, skey_size);
#endif
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  auto hash = stbox::eth::msg_hash(data, data_size);
  sig_size = get_secp256k1_signature_size();
  se_ret = (sgx_status_t)sign_hash(ctx, hash.value(), skey, sig);
#ifdef STBOX_CRYPTO_VERBOSE
  printf("signature size: %u\n", sig_size);
  printf("signature hex: ");
  print_hex(sig, sig_size);
#endif
  secp256k1_context_destroy(ctx);
  return se_ret;
}

uint32_t verify_signature(uint8_t *data, uint32_t data_size, uint8_t *sig,
                          uint32_t sig_size, uint8_t *public_key,
                          uint32_t pkey_size) {
  sgx_status_t se_ret;
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  secp256k1_pubkey secp256k1_pkey;
  memcpy(&secp256k1_pkey, public_key, pkey_size);

  secp256k1_ecdsa_recoverable_signature rsig;
  se_ret = (sgx_status_t)secp256k1_ecdsa_recoverable_signature_parse_compact(
      ctx, &rsig, sig, *(sig + 64) - 27);
  if (!se_ret) {
    return SGX_ERROR_UNEXPECTED;
  }
  secp256k1_ecdsa_signature secp256k1_sig;
  se_ret = (sgx_status_t)secp256k1_ecdsa_recoverable_signature_convert(
      ctx, &secp256k1_sig, &rsig);
  if (!se_ret) {
    return SGX_ERROR_UNEXPECTED;
  }

  auto hash = stbox::eth::msg_hash(data, data_size);
  se_ret = (sgx_status_t)secp256k1_ecdsa_verify(ctx, &secp256k1_sig,
                                                hash.value(), &secp256k1_pkey);
  if (!se_ret) {
    printf("Verify signature failed!\n");
    return SGX_ERROR_UNEXPECTED;
  }
  secp256k1_context_destroy(ctx);
  return SGX_SUCCESS;
}

uint32_t _gen_sgx_ec_key_128bit(const uint8_t *pkey, uint32_t pkey_size,
                                const uint8_t *sealed_key, uint32_t sealed_size,
                                sgx_ec_key_128bit_t *derived_key) {
#ifdef STBOX_CRYPTO_VERBOSE
  printf("########## _gen sgx ec key 128bit ##########\n");
#endif
  sgx_status_t se_ret;
  uint32_t aad_mac_len = strlen(aad_mac_text);
  uint32_t skey_size = sealed_size;
  if (SECP256K1_PRIVATE_KEY_SIZE != sealed_size) {
    skey_size = sgx_get_encrypt_txt_len((sgx_sealed_data_t *)sealed_key);
  }
  uint8_t *skey;
  ff::scope_guard _skey_desc([&]() { skey = new uint8_t[skey_size]; },
                             [&]() { delete[] skey; });
  if (SECP256K1_PRIVATE_KEY_SIZE != sealed_size) {
    se_ret = sgx_unseal_data((const sgx_sealed_data_t *)sealed_key,
                             (uint8_t *)aad_mac_text, (uint32_t *)&aad_mac_len,
                             skey, &skey_size);
    if (se_ret) {
      return se_ret;
    }
  } else {
    memcpy((uint8_t *)skey, sealed_key, sealed_size);
  }
#ifdef STBOX_CRYPTO_VERBOSE
  printf("unseal secret size: %u\n", skey_size);
  printf("unseal secret key hex: ");
  print_hex(skey, skey_size);
#endif

  secp256k1_pubkey secp256k1_pkey;
  memcpy(&secp256k1_pkey, pkey, pkey_size);
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  uint8_t shared_key[skey_size];
  se_ret = (sgx_status_t)secp256k1_ecdh(ctx, shared_key, &secp256k1_pkey, skey,
                                        NULL, NULL);
  if (!se_ret) {
    return SGX_ERROR_UNEXPECTED;
  }
  secp256k1_context_destroy(ctx);
#ifdef STBOX_CRYPTO_VERBOSE
  printf("shared key hex: ");
  print_hex(shared_key, skey_size);
#endif

  sgx_ec256_dh_shared_t ec256_dh_shared_key;
  memcpy(&ec256_dh_shared_key, shared_key, skey_size);
#ifdef STBOX_CRYPTO_VERBOSE
  printf("ec256dh shared key hex: ");
  print_hex(ec256_dh_shared_key.s, skey_size);
#endif
  se_ret = (sgx_status_t)derive_key(&ec256_dh_shared_key, aad_mac_text,
                                    aad_mac_len, derived_key);
#ifdef STBOX_CRYPTO_VERBOSE
  printf("derived key size: %lu\n", SGX_AES_GCM_128BIT_TAG_T_SIZE);
  printf("derived key hex: ");
  print_hex(*derived_key, SGX_AES_GCM_128BIT_TAG_T_SIZE);
#endif
  return se_ret;
}

uint32_t encrypt_message(uint8_t *public_key, uint32_t pkey_size, uint8_t *data,
                         uint32_t data_size, uint8_t *cipher,
                         uint32_t cipher_size) {
#ifdef STBOX_CRYPTO_VERBOSE
  printf("\n########## encrypt message ##########\n");
#endif
  sgx_status_t se_ret;
  uint32_t _pkey_size = get_secp256k1_public_key_size();
  uint32_t _sealed_size = get_secp256k1_sealed_private_key_size();
  uint8_t *_public_key = (uint8_t *)malloc(_pkey_size * sizeof(uint8_t));
  uint8_t *_sealed_key = (uint8_t *)malloc(_sealed_size * sizeof(uint8_t));
  malloc_memory_guard<uint8_t> _pl(_public_key), _sl(_sealed_key);

  se_ret = (sgx_status_t)generate_secp256k1_key_pair(_public_key, _pkey_size,
                                                     _sealed_key, _sealed_size);
  if (se_ret) {
    return se_ret;
  }
  sgx_ec_key_128bit_t derived_key;
  se_ret = (sgx_status_t)_gen_sgx_ec_key_128bit(
      public_key, pkey_size, _sealed_key, _sealed_size, &derived_key);
  if (se_ret) {
    return se_ret;
  }

  sgx_aes_gcm_128bit_tag_t p_out_mac;
  se_ret = sgx_rijndael128GCM_encrypt(
      (const sgx_aes_gcm_128bit_key_t *)derived_key, data, data_size, cipher,
      p_iv_text, INITIALIZATION_VECTOR_SIZE, (const uint8_t *)aad_mac_text,
      strlen(aad_mac_text), &p_out_mac);
  if (se_ret) {
    return se_ret;
  }
  memcpy(cipher + data_size, _public_key, _pkey_size);

  uint32_t sig_size = get_secp256k1_signature_size();
  uint8_t sig[sig_size];
  se_ret = (sgx_status_t)sign_message(_sealed_key, _sealed_size, data,
                                      data_size, sig, sig_size);
  memcpy(cipher + data_size + pkey_size, sig, sig_size);
  memcpy(cipher + data_size + pkey_size + sig_size, p_out_mac,
         SGX_AES_GCM_128BIT_TAG_T_SIZE);
#ifdef STBOX_CRYPTO_VERBOSE
  printf("encrypted msg size: %u\n", cipher_size);
  printf("encrypted msg hex: ");
  print_hex(cipher, cipher_size);
#endif
  return se_ret;
}

uint32_t decrypt_message(uint8_t *sealed_private_key, uint32_t sealed_size,
                         uint8_t *cipher, uint32_t cipher_size, uint8_t *data,
                         uint32_t data_size) {
#ifdef STBOX_CRYPTO_VERBOSE
  printf("\n########## decrypt message ##########\n");
#endif
  sgx_status_t se_ret;
  sgx_ec_key_128bit_t derived_key;
  uint32_t pkey_size = get_secp256k1_public_key_size();
  se_ret = (sgx_status_t)_gen_sgx_ec_key_128bit(cipher + data_size, pkey_size,
                                                sealed_private_key, sealed_size,
                                                &derived_key);
  if (se_ret) {
    return se_ret;
  }
  uint32_t sig_size = get_secp256k1_signature_size();
  se_ret = sgx_rijndael128GCM_decrypt(
      (const sgx_aes_gcm_128bit_key_t *)derived_key, cipher, data_size, data,
      p_iv_text, INITIALIZATION_VECTOR_SIZE, (const uint8_t *)aad_mac_text,
      strlen(aad_mac_text),
      (const sgx_aes_gcm_128bit_tag_t *)(cipher + data_size + pkey_size +
                                         sig_size));
  if (se_ret) {
    return se_ret;
  }
  se_ret = (sgx_status_t)verify_signature(
      data, data_size, cipher + data_size + pkey_size, sig_size,
      cipher + data_size, pkey_size);
#ifdef STBOX_CRYPTO_VERBOSE
  printf("decrypted msg size: %u\n", data_size);
  printf("decrypted msg hex: ");
  print_hex(data, data_size);
#endif
  return se_ret;
}

namespace stbox {
namespace crypto {
uint32_t get_secp256k1_public_key_size() {
  return ::get_secp256k1_public_key_size();
}
uint32_t get_secp256k1_sealed_private_key_size() {
  return ::get_secp256k1_sealed_private_key_size();
}
uint32_t generate_secp256k1_key_pair(uint8_t *pub_key, uint32_t pkey_size,
                                     uint8_t *sealed_priv_key,
                                     uint32_t sealed_size) {
  return ::generate_secp256k1_key_pair(pub_key, pkey_size, sealed_priv_key,
                                       sealed_size);
}
uint32_t generate_secp256k1_pkey_from_skey(const uint8_t *skey,
                                           uint32_t skey_size, uint8_t *pkey,
                                           uint32_t pkey_size) {
  return ::generate_secp256k1_pkey_from_skey(skey, skey_size, pkey, pkey_size);
}

uint32_t get_secp256k1_signature_size() {
  return ::get_secp256k1_signature_size();
}
uint32_t sign_message(const uint8_t *sealed_private_key, uint32_t sealed_size,
                      const uint8_t *data, uint32_t data_size, uint8_t *sig,
                      uint32_t sig_size) {
  return ::sign_message((uint8_t *)sealed_private_key, sealed_size,
                        (uint8_t *)data, data_size, sig, sig_size);
}
uint32_t verify_signature(const uint8_t *data, uint32_t data_size,
                          const uint8_t *sig, uint32_t sig_size,
                          const uint8_t *public_key, uint32_t pkey_size) {
  return ::verify_signature((uint8_t *)data, data_size, (uint8_t *)sig,
                            sig_size, (uint8_t *)public_key, pkey_size);
}

uint32_t get_rijndael128GCM_encrypt_size(uint32_t data_size) {
  return ::get_rijndael128GCM_encrypt_size(data_size);
}
uint32_t encrypt_message(const uint8_t *public_key, uint32_t pkey_size,
                         const uint8_t *data, uint32_t data_size,
                         uint8_t *cipher, uint32_t cipher_size) {
  return ::encrypt_message((uint8_t *)public_key, pkey_size, (uint8_t *)data,
                           data_size, cipher, cipher_size);
}

uint32_t get_rijndael128GCM_decrypt_size(uint32_t cipher_size) {
  return ::get_rijndael128GCM_decrypt_size(cipher_size);
}
uint32_t decrypt_message(const uint8_t *sealed_private_key,
                         uint32_t sealed_size, const uint8_t *cipher,
                         uint32_t cipher_size, uint8_t *data,
                         uint32_t data_size) {
  return ::decrypt_message((uint8_t *)sealed_private_key, sealed_size,
                           (uint8_t *)cipher, cipher_size, data, data_size);
}

} // namespace crypto
} // namespace stbox

