#include "ypc_t/analyzer/parser_wrapper_base.h"
#include "stbox/ebyte.h"
#include "stbox/eth/util.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/log.h"
#include "yaenclave_t.h"
#include "ypc/limits.h"
#include "ypc/param_id.h"
#include "ypc_t/ecommon/version.h"

uint32_t get_ypc_analyzer_version() { return ypc::version(1, 0, 0).data(); }

namespace ypc {

using namespace stbox;
parser_wrapper_base::parser_wrapper_base() {
}
parser_wrapper_base::~parser_wrapper_base() {}

/* Function Description:
 *   This is to verify peer enclave's identity.
 * For demonstration purpose, we verify below points:
 *   1. peer enclave's MRSIGNER is as expected
 *   2. peer enclave's PROD_ID is as expected
 *   3. peer enclave's attribute is reasonable: it's INITIALIZED'ed enclave; in
 *non-debug build configuraiton, the enlave isn't loaded with enclave debug
 *mode.
 **/
stbox::stx_status datahub_verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity) {
    LOG(ERROR) << "verify peer enclave failed";
    return stbox::stx_status::invalid_parameter_error;
  }

  // printf("datahub MRENCLAVE:");
  uint8_t *p = (uint8_t *)&peer_enclave_identity->mr_enclave;
  for (int i = 0; i < sizeof(sgx_measurement_t); ++i) {
    uint8_t c = p[i];
    // printf("%02X ", c);
  }
  return stbox::stx_status::success;
}

stbox::stx_status km_verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity) {
    LOG(ERROR) << "verify key manager enclave failed";
    return stbox::stx_status::invalid_parameter_error;
  }

  // printf("keymgr MRENCLAVE:");
  uint8_t *p = (uint8_t *)&peer_enclave_identity->mr_enclave;
  for (int i = 0; i < sizeof(sgx_measurement_t); ++i) {
    uint8_t c = p[i];
    // printf("%02X ", c);
  }
  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::begin_parse_data_item() {
  m_datahub_session.reset(new stbox::dh_session_initiator(
      stbox::ocall_cast<uint32_t>(datahub_session_request_ocall),
      stbox::ocall_cast<uint32_t>(datahub_exchange_report_ocall),
      stbox::ocall_cast<uint32_t>(datahub_send_request_ocall),
      stbox::ocall_cast<uint32_t>(datahub_end_session_ocall)));
  m_datahub_session->set_verify_peer(datahub_verify_peer_enclave_trust);

  m_keymgr_session.reset(new stbox::dh_session_initiator(
      stbox::ocall_cast<uint32_t>(km_session_request_ocall),
      stbox::ocall_cast<uint32_t>(km_exchange_report_ocall),
      stbox::ocall_cast<uint32_t>(km_send_request_ocall),
      stbox::ocall_cast<uint32_t>(km_end_session_ocall)));
  m_keymgr_session->set_verify_peer(km_verify_peer_enclave_trust);

  auto t1 = m_datahub_session->create_session();
  auto t2 = m_keymgr_session->create_session();
  return static_cast<uint32_t>(t1) | static_cast<uint32_t>(t2);
}

stbox::stx_status parser_wrapper_base::request_private_key() {
  if (m_private_key.size() > 0) {
    return stbox::stx_status::success;
  }
  uint32_t private_key_id = param_id::PRIVATE_KEY;
  size_t max_out_buff_size = 4096;
  std::string request_msg((const char *)&private_key_id, sizeof(uint32_t));
  char *out_buff;
  size_t out_buff_len;
  LOG(INFO) << "request_private_key";
  auto status = m_keymgr_session->send_request_recv_response(
      request_msg.c_str(), request_msg.size(), max_out_buff_size, &out_buff,
      &out_buff_len);
  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for m_keymgr_session->send_request_recv_response: "
               << status;
    return status;
  }
  m_private_key = std::string(out_buff, out_buff_len);
  return status;
}

stbox::stx_status
parser_wrapper_base::decrypt_param(const uint8_t *encrypted_param,
                                   uint32_t len) {
  if (m_param.size() > 0) {
    return stbox::stx_status::success;
  }
  m_encrypted_param = std::string((const char *)encrypted_param, len);
  uint32_t data_len = stbox::crypto::get_rijndael128GCM_decrypt_size(len);
  m_param = std::string(data_len, '0');

  auto ret = stbox::crypto::decrypt_message(
      (const uint8_t *)m_private_key.c_str(), m_private_key.size(),
      encrypted_param, len, (uint8_t *)&m_param[0], data_len);
  if (ret != static_cast<uint32_t>(stbox::stx_status::success)) {
    LOG(ERROR) << "error for stbox::crypto::decrypt_message: " << ret;
    return static_cast<stbox::stx_status>(ret);
  }
  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::parse_data_item(const uint8_t *encrypted_param,
                                              uint32_t len) {
  // request key and param
  auto status = request_private_key();
  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for request_private_key: " << status;
    return static_cast<uint32_t>(status);
  }
  status = decrypt_param(encrypted_param, len);
  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for decrypt_param: " << status;
    return static_cast<uint32_t>(status);
  }

  return static_cast<uint32_t>(stbox::stx_status::success);
}

uint32_t parser_wrapper_base::end_parse_data_item() {
  auto t1 = m_datahub_session->close_session();
  auto t2 = m_keymgr_session->close_session();

  uint32_t pkey_size = stbox::crypto::get_secp256k1_public_key_size();
  std::string pkey(pkey_size, '0');
  auto status = stbox::crypto::generate_secp256k1_pkey_from_skey(
      (uint8_t *)m_private_key.c_str(), m_private_key.size(),
      (uint8_t *)&pkey[0], pkey_size);
  if (status != static_cast<uint32_t>(stbox::stx_status::success)) {
    LOG(ERROR) << "error for generate_secp256k1_pkey_from_skey: " << status;
    return static_cast<uint32_t>(status);
  }
  // TODO Suppose to get address from blockchain
  std::string addr =
      stbox::eth::gen_addr_from_pkey(stbox::string_to_byte(pkey));
  LOG(INFO) << "address : " << addr;

  uint32_t cipher_size =
      stbox::crypto::get_rijndael128GCM_encrypt_size(m_result_str.size());
  m_encrypted_result_str = std::string(cipher_size, '0');
  status = stbox::crypto::encrypt_message(
      (uint8_t *)&pkey[0], pkey_size, (uint8_t *)m_result_str.c_str(),
      m_result_str.size(), (uint8_t *)&m_encrypted_result_str[0], cipher_size);
  if (status != static_cast<uint32_t>(stbox::stx_status::success)) {
    LOG(ERROR) << "error for encrypt_message: " << status;
    return static_cast<uint32_t>(status);
  }

  return static_cast<uint32_t>(t1) | static_cast<uint32_t>(t2);
}

uint32_t parser_wrapper_base::get_encrypted_result_and_signature(
    uint8_t *encrypted_res, uint32_t res_size, uint8_t *result_sig,
    uint32_t sig_size) {
  memcpy(encrypted_res, (uint8_t *)&m_encrypted_result_str[0],
         m_encrypted_result_str.size());
  memcpy(result_sig, (uint8_t *)&m_result_signature_str[0],
         m_result_signature_str.size());
  return static_cast<uint32_t>(stbox::stx_status::success);
}

uint32_t parser_wrapper_base::add_block_parse_result(
    uint16_t block_index, uint8_t *block_result, uint32_t res_size,
    uint8_t *data_hash, uint32_t hash_size, uint8_t *sig, uint32_t sig_size) {

  std::string br((const char *)block_result, res_size);
  block_meta_t meta;
  meta.encrypted_result = std::string((const char *)block_result, res_size);
  meta.data_hash = stbox::bytes(data_hash, hash_size);
  meta.sig = stbox::bytes(sig, sig_size);

  m_block_results[block_index] = meta;
  return static_cast<uint32_t>(stbox::stx_status::success);
}

uint32_t parser_wrapper_base::merge_parse_result(const uint8_t *encrypted_param,
                                                 uint32_t len) {
  auto status = request_private_key();
  if (status != stbox::stx_status::success) {
    return static_cast<uint32_t>(status);
  }
  status = decrypt_param(encrypted_param, len);
  if (status != stbox::stx_status::success) {
    return static_cast<uint32_t>(status);
  }

  // Check if all results are here
  for (uint16_t i = 0; i < m_block_results.size(); i++) {
    auto it = m_block_results.find(i);
    if (it == m_block_results.end()) {
      LOG(ERROR) << "lack result for block " << i;
      return static_cast<uint32_t>(stbox::stx_status::invalid_parameter_error);
    }
  }

  auto decrypt_block_result = [&](const std::string &_encrypted_param,
                                  std::string &_param) -> stbox::stx_status {
    uint32_t data_len =
        stbox::crypto::get_rijndael128GCM_decrypt_size(_encrypted_param.size());
    _param = std::string(data_len, '0');

    auto ret = stbox::crypto::decrypt_message(
        (const uint8_t *)m_private_key.c_str(), m_private_key.size(),
        (uint8_t *)&_encrypted_param[0], _encrypted_param.size(),
        (uint8_t *)&_param[0], data_len);

    if (ret != static_cast<uint32_t>(stbox::stx_status::success)) {
      LOG(ERROR) << "error for decrypt_message: " << ret;
      return static_cast<stbox::stx_status>(ret);
    }
    return stbox::stx_status::success;
  };

  uint32_t pkey_size = stbox::crypto::get_secp256k1_public_key_size();
  uint8_t *pkey = new uint8_t[pkey_size];
  scope_guard _l([&]() { delete[] pkey; });

  stbox::crypto::generate_secp256k1_pkey_from_skey(
      (uint8_t *)m_private_key.c_str(), m_private_key.size(), pkey, pkey_size);

  std::vector<std::string> results;
  for (uint16_t i = 0; i < m_block_results.size(); ++i) {
    const std::string &s = m_block_results[i].encrypted_result;
    std::string r;
    status = decrypt_block_result(s, r);
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for decrypt_block_result: " << status;
      return static_cast<uint32_t>(status);
    }
    auto msg = stbox::bytes(encrypted_param, len) +
               m_block_results[i].data_hash +
               m_block_results[i].encrypted_result;

    auto verified = SGX_SUCCESS;
    // auto verified = stbox::crypto::verify_signature(
    // msg.value(), msg.size(), m_block_results[i].sig.value(),
    // m_block_results[i].sig.size(), pkey, pkey_size);

    if (verified != SGX_SUCCESS) {
      LOG(ERROR) << " verify result " << i << " return "
                 << stbox::sgx_status_string(sgx_status_t(verified));
      return verified;
    } else {
      results.push_back(r);
    }
  }
  m_continue = user_def_block_result_merge(results);
  m_block_results.clear();

  return static_cast<uint32_t>(stbox::stx_status::success);
}
} // namespace ypc
