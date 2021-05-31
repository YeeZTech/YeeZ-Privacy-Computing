#include "common/crypto_prefix.h"
#include "ekeymgr_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

#include "stbox/scope_guard.h"
#include "stbox/stx_common.h"
#include <sgx_report.h>
#include <sgx_tcrypto.h>
#include <sgx_trts.h>
#include <sgx_tseal.h>
#include <sgx_utils.h>

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

uint32_t get_backup_private_key_size(uint32_t sealed_size) {
  if (sealed_size != stbox::crypto::get_secp256k1_sealed_private_key_size()) {
    return SGX_ERROR_UNEXPECTED;
  }

  return stbox::crypto::get_encrypt_message_size_with_prefix(
      stbox::crypto::get_secp256k1_private_key_size());
}

uint32_t backup_private_key(uint8_t *sealed_private_key, uint32_t sealed_size,
                            uint8_t *pub_key, uint32_t pkey_size,
                            uint8_t *backup_private_key, uint32_t bp_size) {

  uint32_t se_ret;
  uint8_t *skey;
  uint32_t skey_size = stbox::crypto::get_secp256k1_private_key_size();
  ff::scope_guard _skey_ptr([&]() { skey = new uint8_t[skey_size]; },
                            [&]() { delete[] skey; });

  se_ret = stbox::crypto::unseal_secp256k1_private_key(sealed_private_key,
                                                       sealed_size, skey);
  if (se_ret) {
    return se_ret;
  }

  se_ret = stbox::crypto::encrypt_message_with_prefix(
      pub_key, pkey_size, skey, skey_size, ypc::utc::crypto_prefix_backup,
      backup_private_key, bp_size);
  return se_ret;
}

uint32_t get_restore_private_key_size(uint32_t bp_size) {
  uint32_t expect = stbox::crypto::get_encrypt_message_size_with_prefix(
      stbox::crypto::get_secp256k1_private_key_size());
  if (bp_size != expect) {
    return SGX_ERROR_UNEXPECTED;
  }
  return stbox::crypto::get_secp256k1_sealed_private_key_size();
}

uint32_t restore_private_key(uint8_t *backup_private_key, uint32_t bp_size,
                             uint8_t *priv_key, uint32_t priv_key_size,
                             uint8_t *sealed_private_key,
                             uint32_t sealed_size) {
  uint32_t se_ret;
  uint8_t *skey;
  uint32_t skey_size = stbox::crypto::get_secp256k1_private_key_size();
  ff::scope_guard _skey_ptr([&]() { skey = new uint8_t[skey_size]; },
                            [&]() { delete[] skey; });

  se_ret = stbox::crypto::unseal_secp256k1_private_key(priv_key, priv_key_size,
                                                       skey);
  if (se_ret) {
    return se_ret;
  }
  uint8_t *raw_bkey;
  uint32_t raw_bkey_size =
      stbox::crypto::get_decrypt_message_size_with_prefix(bp_size);

  ff::scope_guard _raw_bkey_desc(
      [&]() { raw_bkey = new uint8_t[raw_bkey_size]; },
      [&]() { delete[] raw_bkey; });

  se_ret = stbox::crypto::decrypt_message_with_prefix(
      skey, skey_size, backup_private_key, bp_size, raw_bkey, raw_bkey_size,
      ypc::utc::crypto_prefix_backup);
  if (se_ret) {
    return se_ret;
  }
  se_ret = stbox::crypto::seal_secp256k1_private_key(
      raw_bkey, sealed_private_key, sealed_size);
  return se_ret;
}
uint32_t get_forward_private_key_size(uint32_t sealed_size) {
  if (sealed_size != stbox::crypto::get_secp256k1_sealed_private_key_size()) {
    return SGX_ERROR_UNEXPECTED;
  }

  return stbox::crypto::get_encrypt_message_size_with_prefix(
      stbox::crypto::get_secp256k1_private_key_size());
}

uint32_t forward_private_key(uint8_t *sealed_private_key, uint32_t sealed_size,
                             uint8_t *pub_key, uint32_t pkey_size,
                             uint8_t *fwd_private_key, uint32_t fwd_size) {

  uint32_t se_ret;
  uint8_t *skey;
  uint32_t skey_size = stbox::crypto::get_secp256k1_private_key_size();
  ff::scope_guard _skey_ptr([&]() { skey = new uint8_t[skey_size]; },
                            [&]() { delete[] skey; });

  se_ret = stbox::crypto::unseal_secp256k1_private_key(sealed_private_key,
                                                       sealed_size, skey);
  if (se_ret) {
    LOG(ERROR) << "unseal_secp256k1_private_key returns: " << se_ret;
    return se_ret;
  }

  se_ret = stbox::crypto::encrypt_message_with_prefix(
      pub_key, pkey_size, skey, skey_size, ypc::utc::crypto_prefix_forward,
      fwd_private_key, fwd_size);
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

uint32_t msession_request(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id) {
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
uint32_t mexchange_report(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
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
                                    uint8_t *skey_ptr, uint32_t *skey_size) {
  *skey_size = stbox::crypto::get_secp256k1_private_key_size();
  if (!skey_ptr) {
    return SGX_SUCCESS;
  }

  uint32_t sealed_size = get_secp256k1_sealed_private_key_size();
  uint8_t *sealed_key;
  ff::scope_guard _sealed_key_ptr_desc(
      [&]() { sealed_key = new uint8_t[sealed_size]; },
      [&]() { delete[] sealed_key; });

  std::string key_path(".yeez.key/");
  uint32_t ret = stbox::ocall_cast<uint32_t>(ocall_load_key_pair)(
      key_path.c_str(), key_path.size(), pkey_ptr, pkey_size, sealed_key,
      sealed_size);
  if (ret != 0) {
    LOG(ERROR) << "failed to load key pair: " << stbox::status_string(ret);
    return ret;
  }

  ret = stbox::crypto::unseal_secp256k1_private_key(sealed_key, sealed_size,
                                                    skey_ptr);
  return ret;
}

uint32_t load_and_check_key_pair(const uint8_t *pkey, uint32_t pkey_size,
                                 stbox::bytes &skey) {
  uint32_t skey_size;
  auto se_ret = (sgx_status_t)load_key_pair_if_not_exist(
      (uint8_t *)pkey, pkey_size, nullptr, &skey_size);
  if (se_ret) {
    return se_ret;
  }
  skey = stbox::bytes(skey_size);

  se_ret = (sgx_status_t)load_key_pair_if_not_exist((uint8_t *)pkey, pkey_size,
                                                    skey.data(), &skey_size);
  if (se_ret) {
    return se_ret;
  }

  stbox::bytes expect_pkey(stbox::crypto::get_secp256k1_public_key_size());
  stbox::crypto::generate_secp256k1_pkey_from_skey(
      skey.data(), expect_pkey.data(), expect_pkey.size());
  if (memcmp(expect_pkey.data(), pkey, expect_pkey.size()) != 0) {
    return stbox::stx_status::kmgr_pkey_skey_mismatch;
  }
  return stbox::stx_status::success;
}

std::unordered_map<stbox::bytes, forward_message_st> message_table;

uint32_t forward_message(uint32_t msg_id, uint8_t *cipher, uint32_t cipher_size,
                         uint8_t *epublic_key, uint32_t epkey_size,
                         uint8_t *ehash, uint32_t ehash_size,
                         uint8_t *verify_key, uint32_t vpkey_size, uint8_t *sig,
                         uint32_t sig_size) {
  uint32_t se_ret = SGX_SUCCESS;

  uint32_t msg_key_size = sizeof(msg_id) + ehash_size;
  stbox::bytes str_msg_key(msg_key_size);
  memcpy((uint8_t *)str_msg_key.data(), &msg_id, sizeof(msg_id));
  memcpy((uint8_t *)str_msg_key.data() + sizeof(msg_id), ehash, ehash_size);

  if (message_table.find(str_msg_key) != message_table.end()) {
    LOG(WARNING) << "key already exist";
    return se_ret;
  }

  stbox::bytes skey;
  se_ret = load_and_check_key_pair(epublic_key, epkey_size, skey);
  if (se_ret) {
    return se_ret;
  }

  uint32_t decrypted_size =
      ::stbox::crypto::get_decrypt_message_size_with_prefix(cipher_size);
  stbox::bytes decrypted_msg(decrypted_size);
  se_ret = (sgx_status_t)::stbox::crypto::decrypt_message_with_prefix(
      skey.data(), skey.size(), cipher, cipher_size, decrypted_msg.data(),
      decrypted_size, ::ypc::utc::crypto_prefix_forward);

  if (se_ret) {
    LOG(ERROR) << "decrypt_message_with_prefix forward returns " << se_ret;
    return se_ret;
  }

  uint32_t all_size = sizeof(msg_id) + cipher_size + epkey_size + ehash_size;
  stbox::bytes all(all_size);
  memcpy(all.data(), &msg_id, sizeof(msg_id));
  memcpy(all.data() + sizeof(msg_id), cipher, cipher_size);
  memcpy(all.data() + sizeof(msg_id) + cipher_size, epublic_key, epkey_size);
  memcpy(all.data() + sizeof(msg_id) + cipher_size + epkey_size, ehash,
         ehash_size);
  se_ret = (sgx_status_t)verify_signature(all.data(), all.size(), sig, sig_size,
                                          verify_key, vpkey_size);
  if (se_ret) {
    LOG(ERROR) << "Invalid signature";
    return se_ret;
  }

  message_table.insert(std::make_pair(
      str_msg_key,
      forward_message_st{msg_id, decrypted_msg, bytes(ehash, ehash_size)}));
  return se_ret;
}

stbox::bytes handle_pkg(const uint8_t *data, size_t data_len,
                        stbox::dh_session *context) {
  uint32_t msg_key_size = data_len + sizeof(sgx_measurement_t);
  stbox::bytes str_msg_key(msg_key_size);
  memcpy(&str_msg_key[0], data, data_len);
  memcpy(&str_msg_key[data_len], &context->peer_identity().mr_enclave,
         sizeof(sgx_measurement_t));
  LOG(INFO) << "got request: " << str_msg_key;

  auto iter = message_table.find(str_msg_key);
  if (iter != message_table.end()) {
    return iter->second.m_msg;
  }
  LOG(ERROR) << "Request message not exist in message table!";
  throw std::runtime_error("Request message not exist in message table!");
}

uint32_t mgenerate_response(secure_message_t *req_message,
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
uint32_t mend_session(uint32_t session_id) {
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
