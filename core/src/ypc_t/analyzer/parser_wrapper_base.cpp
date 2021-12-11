#include "ypc_t/analyzer/parser_wrapper_base.h"
#include "common/crypto_prefix.h"
#include "common/limits.h"
#include "common/param_id.h"
#include "corecommon/package.h"
#include "stbox/ebyte.h"
#include "stbox/eth/util.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/log.h"
#include "yaenclave_t.h"
#include "ypc_t/ecommon/signer_verify.h"
#include "ypc_t/ecommon/version.h"

uint32_t get_ypc_analyzer_version() { return ypc::version(1, 0, 0).data(); }

namespace ypc {

using namespace stbox;
parser_wrapper_base::parser_wrapper_base() {
}
parser_wrapper_base::~parser_wrapper_base() {}

stbox::stx_status datahub_verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity) {
    LOG(ERROR) << "verify datahub enclave failed";
    return stbox::stx_status::invalid_parameter_error;
  }

  return stbox::stx_status::success;
}

stbox::stx_status km_verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity) {
    LOG(ERROR) << "verify key manager enclave failed";
    return stbox::stx_status::invalid_parameter_error;
  }

  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::begin_parse_data_item() {
  LOG(INFO) << "begin_parse_data_item()";
  m_datahub_session.reset(new stbox::dh_session_initiator(
      stbox::ocall_cast<uint32_t>(datahub_session_request_ocall),
      stbox::ocall_cast<uint32_t>(datahub_exchange_report_ocall),
      stbox::ocall_cast<uint32_t>(datahub_send_request_ocall),
      stbox::ocall_cast<uint32_t>(datahub_end_session_ocall)));
  m_datahub_session->set_verify_peer(datahub_verify_peer_enclave_trust);

  LOG(INFO) << "done init datahub session";

  m_keymgr_session.reset(new stbox::dh_session_initiator(
      stbox::ocall_cast<uint32_t>(km_session_request_ocall),
      stbox::ocall_cast<uint32_t>(km_exchange_report_ocall),
      stbox::ocall_cast<uint32_t>(km_send_request_ocall),
      stbox::ocall_cast<uint32_t>(km_end_session_ocall)));
  m_keymgr_session->set_verify_peer(km_verify_peer_enclave_trust);

  LOG(INFO) << "done init keymgr session";

  auto t1 = m_datahub_session->create_session();
  LOG(INFO) << "done create datahub session";
  auto t2 = m_keymgr_session->create_session();
  LOG(INFO) << "done create keymgr session";
  return t1 | t2;
}

uint32_t parser_wrapper_base::request_private_key() {
  if (m_private_key.size() > 0) {
    return stbox::stx_status::success;
  }
  uint32_t private_key_id = param_id::PRIVATE_KEY;
  /*
  std::string request_msg((const char *)&private_key_id, sizeof(uint32_t));
  */

  LOG(INFO) << "request_private_key()";
  stbox::bytes request_msg = ypc::make_bytes<stbox::bytes>::for_package<
      request_private_key_pkg_t, nt<stbox::bytes>::id>(private_key_id);

  auto status = m_keymgr_session->send_request_recv_response(
      (char *)request_msg.data(), request_msg.size(),
      utc::max_keymgr_response_buf_size, m_private_key);

  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for m_keymgr_session->send_request_recv_response: "
               << stbox::status_string(status);
    return status;
  }
  LOG(INFO) << "request private key done";

  uint32_t pkey_size = stbox::crypto::get_secp256k1_public_key_size();
  m_pkey4v = stbox::bytes(pkey_size);
  status = (stbox::stx_status)stbox::crypto::generate_secp256k1_pkey_from_skey(
      (const uint8_t *)&m_private_key[0], (uint8_t *)&m_pkey4v[0], pkey_size);
  if (status) {
    LOG(ERROR) << "error for generate_secp256k1_pkey_from_skey: " << status;
    return status;
  }
  LOG(INFO) << "using private key whose public key is: " << m_pkey4v;

  return status;
}

uint32_t parser_wrapper_base::decrypt_param(const uint8_t *encrypted_param,
                                            uint32_t len) {
  if (m_param.size() > 0) {
    return stbox::stx_status::success;
  }
  LOG(INFO) << "start decrypt param";
  m_encrypted_param = bytes((const char *)encrypted_param, len);
  uint32_t data_len = stbox::crypto::get_decrypt_message_size_with_prefix(len);
  m_param = bytes(data_len);

  auto ret = stbox::crypto::decrypt_message_with_prefix(
      (const uint8_t *)m_private_key.data(), m_private_key.size(),
      encrypted_param, len, (uint8_t *)&m_param[0], data_len,
      utc::crypto_prefix_arbitrary);
  if (ret) {
    LOG(ERROR) << "error for stbox::crypto::decrypt_message: " << ret;
    return ret;
  }
  LOG(INFO) << "end decrypt param";
  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::parse_data_item(const uint8_t *encrypted_param,
                                              uint32_t len) {
  // request key and param
  auto status = request_private_key();
  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for request_private_key: "
               << stbox::status_string(status)
               << ", this is caused by using wrong enclave or wrong encrypted "
                  "private key";
    return status;
  }
  LOG(INFO) << "Done checking enclave hash";

  status = decrypt_param(encrypted_param, len);
  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for decrypt_param: " << stbox::status_string(status)
               << ", this is caused by using wrong parameters";
    return status;
  }
  LOG(INFO) << "Done checking parameters";

  status = request_extra_data_usage();
  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for request_extra_data_usage: "
               << stbox::status_string(status);
    return status;
  }

  LOG(INFO) << "Done checking extra data usage license";
  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::end_parse_data_item() {
  auto t1 = m_datahub_session->close_session();
  auto t2 = m_keymgr_session->close_session();

  uint32_t cipher_size =
      stbox::crypto::get_encrypt_message_size_with_prefix(m_result_str.size());
  m_encrypted_result_str = bytes(cipher_size);

  auto status = stbox::crypto::encrypt_message_with_prefix(
      (const uint8_t *)&m_pkey4v[0], m_pkey4v.size(),
      (const uint8_t *)m_result_str.data(), m_result_str.size(),
      utc::crypto_prefix_arbitrary, (uint8_t *)&m_encrypted_result_str[0],
      cipher_size);
  if (status != stbox::stx_status::success) {
    LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(status);
    return status;
  }

  return t1 | t2;
}

uint32_t
parser_wrapper_base::get_analyze_result(parser_wrapper_base::result_t &res) {
  using ntt = ypc::nt<stbox::bytes>;

  res.set<ntt::data_hash>(data_hash());
  res.set<ntt::encrypted_result>(m_encrypted_result_str);
  res.set<ntt::result_signature>(m_result_signature_str);
  res.set<ntt::cost_signature>(m_cost_signature_str);
  res.set<ntt::result_encrypt_key>(get_result_encrypt_key());
  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::add_block_parse_result(
    uint16_t block_index, uint8_t *block_result, uint32_t res_size,
    uint8_t *data_hash, uint32_t hash_size, uint8_t *sig, uint32_t sig_size) {

  block_meta_t meta;
  meta.encrypted_result = stbox::bytes(block_result, res_size);
  meta.data_hash = stbox::bytes(data_hash, hash_size);
  meta.sig = stbox::bytes(sig, sig_size);

  m_block_results[block_index] = meta;
  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::merge_parse_result(const uint8_t *encrypted_param,
                                                 uint32_t len) {
  auto status = request_private_key();
  if (status != stbox::stx_status::success) {
    return status;
  }
  status = decrypt_param(encrypted_param, len);
  if (status != stbox::stx_status::success) {
    return status;
  }

  // Check if all results are here
  for (uint16_t i = 0; i < m_block_results.size(); i++) {
    auto it = m_block_results.find(i);
    if (it == m_block_results.end()) {
      LOG(ERROR) << "lack result for block " << i;
      return stbox::stx_status::invalid_parameter_error;
    }
  }

  auto decrypt_block_result = [&](const stbox::bytes &_encrypted_param,
                                  stbox::bytes &_param) -> uint32_t {
    uint32_t data_len = stbox::crypto::get_decrypt_message_size_with_prefix(
        _encrypted_param.size());
    _param = stbox::bytes(data_len);

    auto ret = stbox::crypto::decrypt_message_with_prefix(
        (const uint8_t *)m_private_key.data(), m_private_key.size(),
        (uint8_t *)&_encrypted_param[0], _encrypted_param.size(),
        (uint8_t *)&_param[0], data_len, utc::crypto_prefix_arbitrary);

    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for decrypt_message: " << stbox::status_string(ret);
      return ret;
    }
    return stbox::stx_status::success;
  };

  uint32_t pkey_size = stbox::crypto::get_secp256k1_public_key_size();
  uint8_t *pkey = new uint8_t[pkey_size];
  scope_guard _l([&]() { delete[] pkey; });

  stbox::crypto::generate_secp256k1_pkey_from_skey(
      (uint8_t *)m_private_key.data(), pkey, pkey_size);

  std::vector<stbox::bytes> results;
  for (uint16_t i = 0; i < m_block_results.size(); ++i) {
    const stbox::bytes &s = m_block_results[i].encrypted_result;
    stbox::bytes r;
    status = decrypt_block_result(s, r);
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for decrypt_block_result: "
                 << stbox::status_string(status);
      return status;
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
                 << stbox::status_string(verified);
      return verified;
    } else {
      results.push_back(r);
    }
  }
  m_continue = user_def_block_result_merge(results);
  m_block_results.clear();
  if (m_enclave_hash.size() == 0) {
    LOG(WARNING) << "didn't set enclave hash";
  }

  return stbox::stx_status::success;
}
} // namespace ypc
