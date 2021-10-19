#include "common/crypto_prefix.h"
#include "common/endian.h"
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

#include "keymgr/common/message_type.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/tsgx/channel/dh_session_responder.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/crypto/ecp_interface.h"
#include "stbox/tsgx/log.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"
#include "ypc_t/ecommon/signer_verify.h"

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


uint32_t get_encrypted_result_and_signature(
    uint8_t *_encrypted_param, uint32_t encrypted_param_size,
    uint8_t *_enclave_hash, uint32_t enclave_hash_size, uint8_t *_result,
    uint32_t result_size, uint8_t *_private_key, uint32_t private_key_size,
    uint8_t *_data_hash, uint32_t data_hash_size, uint64_t cost,
    uint8_t *_encrypted_res, uint32_t res_size, uint8_t *_result_sig,
    uint32_t sig_size, uint8_t *_cost_sig, uint32_t cost_sig_size) {

  stbox::bytes cost_gas_str(sizeof(cost));
  memcpy((uint8_t *)&cost_gas_str[0], (uint8_t *)&cost, sizeof(cost));
  LOG(INFO) << "cost gas before change_endian: " << cost_gas_str;
  ypc::utc::endian_swap(cost_gas_str.data(), 0, sizeof(cost));
  LOG(INFO) << "cost gas after change_endian: " << cost_gas_str;
  auto encrypted_param = stbox::bytes(_encrypted_param, encrypted_param_size);
  auto enclave_hash = stbox::bytes(_enclave_hash, enclave_hash_size);
  auto result = stbox::bytes(_result, result_size);
  auto data_hash = stbox::bytes(_data_hash, data_hash_size);

  uint32_t cipher_size =
      stbox::crypto::get_encrypt_message_size_with_prefix(result.size());

  uint8_t pkey[64];
  ::stbox::crypto::generate_secp256k1_pkey_from_skey(_private_key, pkey, 64);
  LOG(INFO) << "pkey: " << stbox::bytes(pkey, 64);

  auto status = stbox::crypto::encrypt_message_with_prefix(
      (const uint8_t *)&pkey[0], 64, (const uint8_t *)result.data(),
      result.size(), ::ypc::utc::crypto_prefix_arbitrary, _encrypted_res,
      res_size);

  auto cost_msg = encrypted_param + data_hash + enclave_hash + cost_gas_str;
  LOG(INFO) << "input: " << encrypted_param;
  LOG(INFO) << "data_hash: " << data_hash;
  LOG(INFO) << "enclave_hash: " << enclave_hash;
  LOG(INFO) << "cost message: " << cost_msg;

  status = stbox::crypto::sign_message(_private_key, private_key_size,
                                       (uint8_t *)&cost_msg[0], cost_msg.size(),
                                       _cost_sig, cost_sig_size);

  auto msg = cost_msg + result;
  status = stbox::crypto::sign_message(_private_key, private_key_size,
                                       (uint8_t *)&msg[0], msg.size(),
                                       _result_sig, sig_size);
  return status;
}

uint32_t get_encrypt_message_size_with_prefix(uint32_t data_size) {
  return ::stbox::crypto::get_encrypt_message_size_with_prefix(data_size);
}