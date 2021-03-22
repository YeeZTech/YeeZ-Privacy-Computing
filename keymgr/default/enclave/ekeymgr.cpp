#include "ekeymgr_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

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
#include "stbox/tsgx/log.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"
#include "ypc_t/ecommon/signer_verify.h"

#define SECP256K1_PRIVATE_KEY_SIZE 32
#define INITIALIZATION_VECTOR_SIZE 12
#define SGX_AES_GCM_128BIT_TAG_T_SIZE sizeof(sgx_aes_gcm_128bit_tag_t)

extern "C" {
#include "stbox/keccak/keccak.h"
}

using stx_status = stbox::stx_status;
using scope_guard = stbox::scope_guard;
using namespace stbox;
// using namespace stbox::crypto;

static char aad_mac_text[64] = "tech.yeez.key.manager";
// uint8_t *p_iv_text =
//(uint8_t *)malloc(INITIALIZATION_VECTOR_SIZE * sizeof(uint8_t));

uint32_t backup_private_key(uint8_t *sealed_private_key, uint32_t sealed_size,
                            uint8_t *pub_key, uint32_t pkey_size,
                            uint8_t *backup_private_key, uint32_t bp_size) {
#ifdef DEBUG_LOG
  printf("\n########## backup private key ##########\n");
#endif
  sgx_status_t se_ret;
  uint32_t data_size =
      sgx_get_encrypt_txt_len((sgx_sealed_data_t *)sealed_private_key);
  uint8_t *data;
  ff::scope_guard _l([&]() { data = new uint8_t[data_size]; },
                     [&]() { delete[] data; });
  uint32_t aad_mac_len = strlen(aad_mac_text);
  se_ret = sgx_unseal_data((const sgx_sealed_data_t *)sealed_private_key,
                           (uint8_t *)aad_mac_text, (uint32_t *)&aad_mac_len,
                           data, &data_size);
  if (se_ret) {
    return se_ret;
  }
#ifdef DEBUG_LOG
  printf("unseal data size: %u\n", data_size);
  printf("unseal data hex: ");
  print_hex(data, data_size);
#endif

  se_ret = (sgx_status_t)encrypt_message(pub_key, pkey_size, data, data_size,
                                         backup_private_key, bp_size);
#ifdef DEBUG_LOG
  printf("backup private key size: %u\n", bp_size);
  printf("backup private key hex: ");
  print_hex(backup_private_key, bp_size);
#endif
  return se_ret;
}

uint32_t restore_private_key(uint8_t *backup_private_key, uint32_t bp_size,
                             uint8_t *priv_key, uint32_t skey_size,
                             uint8_t *sealed_private_key,
                             uint32_t sealed_size) {
#ifdef DEBUG_LOG
  printf("\n########## restore private key ##########\n");
#endif
  sgx_status_t se_ret;
  uint8_t data[SECP256K1_PRIVATE_KEY_SIZE];
  uint32_t aad_mac_len = strlen(aad_mac_text);
  se_ret =
      (sgx_status_t)decrypt_message(priv_key, skey_size, backup_private_key,
                                    bp_size, data, SECP256K1_PRIVATE_KEY_SIZE);
  if (se_ret) {
    return se_ret;
  }
#ifdef DEBUG_LOG
  printf("restore private key size: %u\n", SECP256K1_PRIVATE_KEY_SIZE);
  printf("restore private key hex: ");
  print_hex(data, SECP256K1_PRIVATE_KEY_SIZE);
#endif

  se_ret = sgx_seal_data(
      (const uint32_t)strlen(aad_mac_text), (const uint8_t *)aad_mac_text,
      SECP256K1_PRIVATE_KEY_SIZE, data, (const uint32_t)sealed_size,
      (sgx_sealed_data_t *)sealed_private_key);
#ifdef DEBUG_LOG
  printf("sealed secret key size: %d\n", sealed_size);
  printf("sealed secret key hex: ");
  print_hex(sealed_private_key, sealed_size);
#endif
  return se_ret;
}

std::shared_ptr<stbox::dh_session_responder> dh_resp_session(nullptr);
stbox::stx_status verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity) {
    LOG(ERROR) << "(!peer_enclave_identity): invalid peer enclave identity";
    return stbox::stx_status::invalid_parameter_error;
  }
  // TODO, we should hard-code this signer
  if (!ypc::is_certified_signer(peer_enclave_identity)) {
    return stbox::stx_status::enclave_trust_error;
  }
  return stbox::stx_status::success;
}
uint32_t session_request(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
      dh_resp_session->set_verify_peer(verify_peer_enclave_trust);
    }
    return static_cast<uint32_t>(
        dh_resp_session->session_request(dh_msg1, session_id));

  } catch (std::exception &e) {
    LOG(ERROR) << "session_request get exception " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}
uint32_t exchange_report(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
                         uint32_t session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
    }
    return static_cast<uint32_t>(
        dh_resp_session->exchange_report(dh_msg2, dh_msg3, session_id));

  } catch (std::exception &e) {
    LOG(ERROR) << "exchange_report get exception " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}

uint32_t load_key_pair_if_not_exist(uint8_t *pkey_ptr, uint32_t pkey_size,
                                    uint8_t **skey_ptr, uint32_t *sealed_size) {
  *sealed_size = get_secp256k1_sealed_private_key_size();
  *skey_ptr = new uint8_t[*sealed_size];
  return stbox::ocall_cast<uint32_t>(ocall_load_key_pair)(
      pkey_ptr, pkey_size, *skey_ptr, *sealed_size);
}

std::unordered_map<std::string, forward_message_st> message_table;

uint32_t forward_message(uint32_t msg_id, uint8_t *cipher, uint32_t cipher_size,
                         uint8_t *epublic_key, uint32_t epkey_size,
                         uint8_t *ehash, uint32_t ehash_size,
                         uint8_t *verify_key, uint32_t vpkey_size, uint8_t *sig,
                         uint32_t sig_size) {
  sgx_status_t se_ret = SGX_SUCCESS;

  uint32_t msg_key_size = sizeof(msg_id) + ehash_size;
  std::string str_msg_key(msg_key_size, '0');
  memcpy((uint8_t *)str_msg_key.c_str(), &msg_id, sizeof(msg_id));
  memcpy((uint8_t *)str_msg_key.c_str() + sizeof(msg_id), ehash, ehash_size);

  if (message_table.find(str_msg_key) != message_table.end()) {
    LOG(WARNING) << "key already exist";
    return se_ret;
  }

  uint8_t *skey_ptr;
  uint32_t sealed_size;
  se_ret = (sgx_status_t)load_key_pair_if_not_exist(epublic_key, epkey_size,
                                                    &skey_ptr, &sealed_size);
  ff::scope_guard _skey_ptr_desc([&]() { delete[] skey_ptr; });

  uint32_t all_size = sizeof(msg_id) + cipher_size + epkey_size + ehash_size;
  stbox::bytes all(all_size);
  memcpy(all.value(), &msg_id, sizeof(msg_id));
  memcpy(all.value() + sizeof(msg_id), cipher, cipher_size);
  memcpy(all.value() + sizeof(msg_id) + cipher_size, epublic_key, epkey_size);
  memcpy(all.value() + sizeof(msg_id) + cipher_size + epkey_size, ehash,
         ehash_size);
  se_ret = (sgx_status_t)verify_signature(all.value(), all.size(), sig,
                                          sig_size, verify_key, vpkey_size);
  if (se_ret) {
    LOG(ERROR) << "Invalid signature";
    printf("Invalid signature!\n");
    return se_ret;
  }
  uint32_t decrypted_size = get_rijndael128GCM_decrypt_size(cipher_size);
  stbox::bytes decrypted_msg(decrypted_size);
  se_ret =
      (sgx_status_t)decrypt_message(skey_ptr, sealed_size, cipher, cipher_size,
                                    decrypted_msg.value(), decrypted_size);
  if (se_ret) {
    LOG(ERROR) << "decrypt_message returns " << se_ret;
    return se_ret;
  }

  std::string str_msg((const char *)decrypted_msg.value(), decrypted_size);
  std::string str_hash((const char *)ehash, ehash_size);
  message_table.insert(std::make_pair(
      str_msg_key, forward_message_st{msg_id, str_msg, str_hash}));
  LOG(INFO) << "recv msg ";
  return se_ret;
}

std::string handle_pkg(const uint8_t *data, size_t data_len,
                       stbox::dh_session *context) {
  uint32_t msg_key_size = data_len + sizeof(sgx_measurement_t);
  std::string str_msg_key(msg_key_size, '0');
  memcpy(&str_msg_key[0], data, data_len);
  memcpy(&str_msg_key[data_len], &context->peer_identity().mr_enclave,
         sizeof(sgx_measurement_t));

  auto iter = message_table.find(str_msg_key);
  if (iter != message_table.end()) {
    return iter->second.m_msg;
  }
  LOG(ERROR) << "Request message not exist in message table!";
  throw std::runtime_error("Request message not exist in message table!");
}

uint32_t generate_response(secure_message_t *req_message,
                           size_t req_message_size, size_t max_payload_size,
                           secure_message_t *resp_message,
                           size_t resp_message_size, uint32_t session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
    }
    return static_cast<uint32_t>(dh_resp_session->generate_response(
        req_message, req_message_size, handle_pkg, max_payload_size,
        resp_message, resp_message_size, session_id));

  } catch (std::exception &e) {
    LOG(ERROR) << "generate_response get exception " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}
uint32_t end_session(uint32_t session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
    }
    return static_cast<uint32_t>(dh_resp_session->end_session(session_id));

  } catch (std::exception &e) {
    LOG(ERROR) << "end_session get exception " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}
