#include "enclave_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

#include "sgx_ecp_types.h"
#include "sgx_tcrypto.h"
#include "stbox/scope_guard.h"
#include "stbox/stx_common.h"
#include <sgx_tcrypto.h>
#include <sgx_trts.h>
#include <sgx_tseal.h>

#include "corecommon/crypto/stdeth.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/tsgx/channel/dh_session_responder.h"
#include "stbox/tsgx/log.h"
#include "ypc_t/ecommon/signer_verify.h"

using ecc = ypc::crypto::eth_sgx_crypto;
using raw_ecc = ecc;
#define SECP256K1_PRIVATE_KEY_SIZE 32
#define INITIALIZATION_VECTOR_SIZE 12
#define SGX_AES_GCM_128BIT_TAG_T_SIZE sizeof(sgx_aes_gcm_128bit_tag_t)

extern "C" {
#include "stbox/../../src/tsgx/secp256k1/hash.h"
#include "stbox/keccak/keccak.h"
}

using stx_status = stbox::stx_status;
using scope_guard = stbox::scope_guard;
using namespace stbox;
// using namespace stbox::crypto;


uint32_t aes_cmac_msg(uint8_t *p_key, uint8_t *p_src, uint32_t src_len,
                      uint8_t *p_mac) {
  return sgx_rijndael128_cmac_msg((sgx_cmac_128bit_key_t *)p_key, p_src,
                                  src_len, (sgx_cmac_128bit_tag_t *)p_mac);
}

uint32_t aes_gcm_encrypt(uint8_t *key, uint8_t *data, uint32_t data_size,
                         uint8_t *cipher, uint8_t *iv, uint32_t iv_size,
                         uint8_t *aad, uint32_t aad_size, uint8_t *mac) {

  return sgx_rijndael128GCM_encrypt((const sgx_aes_gcm_128bit_key_t *)key, data,
                                    data_size, cipher, iv, iv_size, aad,
                                    aad_size, (sgx_aes_gcm_128bit_tag_t *)mac);
}


uint32_t test_ecdh(uint8_t *skey, uint8_t *pkey, uint8_t *shared_key) {

#if 0
  printf("test_ecdh::skey: ");
  print_hex(skey, 32);
  printf("test_ecdh::pkey: ");
  print_hex(pkey, 64);
#endif
  // ypc::change_endian(pkey, 64); // convert to little
  return ecc::ecdh_t::ecdh_shared_key(skey, 32, pkey, 64, shared_key, 16);

#if 0
  printf("test_ecdh::shared_key: ");
  print_hex(shared_key, 32);
#endif
}

uint32_t test_generate_pkey(uint8_t *skey, uint8_t *pkey) {
  return raw_ecc::generate_pkey_from_skey(skey, 32, pkey, 64);
}

uint32_t encrypt_message_with_prefix(uint8_t *public_key, uint32_t prefix,
                                     uint8_t *data, uint32_t data_size,
                                     uint8_t *cipher, uint32_t cipher_size) {
  return raw_ecc::encrypt_message_with_prefix(public_key, 64, data, data_size,
                                              prefix, cipher, cipher_size);
}

uint32_t decrypt_message_with_prefix(uint8_t *skey, uint32_t prefix,
                                     uint8_t *cipher, uint32_t cipher_size,
                                     uint8_t *data, uint32_t data_size) {
  return raw_ecc::decrypt_message_with_prefix(skey, 32, cipher, cipher_size,
                                              data, data_size, prefix);
}

uint32_t get_encrypt_message_size_with_prefix(uint32_t data_size) {
  return raw_ecc::get_encrypt_message_size_with_prefix(data_size);
}
uint32_t get_decrypt_message_size_with_prefix(uint32_t data_size) {
  return raw_ecc::get_decrypt_message_size_with_prefix(data_size);
}
uint32_t test_sign_message(uint8_t *skey, uint32_t skey_size, uint8_t *data,
                           uint32_t data_size, uint8_t *sig) {
  return raw_ecc::sign_message(skey, skey_size, data, data_size, sig, 65);
}

uint32_t test_verify_message(uint8_t *data, uint32_t data_size, uint8_t *sig,
                             uint32_t sig_size, uint8_t *pkey) {

  return raw_ecc::verify_signature(data, data_size, sig, sig_size, pkey, 64);
}

