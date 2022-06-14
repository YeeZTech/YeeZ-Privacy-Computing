#include "keymgr_sgx_module.h"
#include "ekeymgr_u.h"
#include "sgx_urts.h"
#include <glog/logging.h>
#include <stdexcept>

keymgr_sgx_module::keymgr_sgx_module(const char *mod_path)
    : ::stbox::sgx_module(mod_path) {}
keymgr_sgx_module::~keymgr_sgx_module() {}

uint32_t keymgr_sgx_module::get_secp256k1_sealed_private_key_size() {
  return ecall<uint32_t>(::get_secp256k1_sealed_private_key_size);
}
uint32_t
keymgr_sgx_module::generate_secp256k1_key_pair(bref &_pkey,
                                               bref &_sealed_private_key) {
  uint8_t *public_key;
  uint32_t pkey_size;
  uint8_t *sealed_private_key;
  uint32_t sealed_size;
  stbox::buffer_length_t buf_pub(&pkey_size, &public_key,
                                 get_secp256k1_public_key_size);
  stbox::buffer_length_t buf_sec(&sealed_size, &sealed_private_key,
                                 ::get_secp256k1_sealed_private_key_size);
  auto t = ecall<uint32_t>(::generate_secp256k1_key_pair, stbox::xmem(buf_pub),
                           stbox::xlen(buf_pub), stbox::xmem(buf_sec),
                           stbox::xlen(buf_sec));

  _pkey = bref(public_key, pkey_size);
  _sealed_private_key = bref(sealed_private_key, sealed_size);
  return t;
}

uint32_t keymgr_sgx_module::sign_message(const uint8_t *sealed_private_key,
                                         uint32_t sealed_size,
                                         const uint8_t *data,
                                         uint32_t data_size, bref &_sig) {
  uint8_t *sig;
  uint32_t sig_size;
  stbox::buffer_length_t buf_sig(&sig_size, &sig, get_secp256k1_signature_size);
  auto t = ecall<uint32_t>(::sign_message, (uint8_t *)sealed_private_key,
                           sealed_size, (uint8_t *)data, data_size,
                           stbox::xmem(buf_sig), stbox::xlen(buf_sig));

  _sig = bref(sig, sig_size);
  return t;
}

uint32_t keymgr_sgx_module::verify_signature(
    const uint8_t *data, uint32_t data_size, const uint8_t *sig,
    uint32_t sig_size, const uint8_t *public_key, uint32_t pkey_size) {
  return ecall<uint32_t>(::verify_signature, (uint8_t *)data, data_size,
                         (uint8_t *)sig, sig_size, (uint8_t *)public_key,
                         pkey_size);
}

uint32_t keymgr_sgx_module::encrypt_message(const uint8_t *public_key,
                                            uint32_t pkey_size,
                                            const uint8_t *data,
                                            uint32_t data_size,
                                            ypc::bref &_cipher) {

  uint8_t *cipher;
  uint32_t cipher_size;

  stbox::buffer_length_t buf_cip(&cipher_size, &cipher,
                                 ::get_encrypted_message_size, data_size);

  auto t = ecall<uint32_t>(::encrypt_message, (uint8_t *)public_key, pkey_size,
                           (uint8_t *)data, data_size, stbox::xmem(buf_cip),
                           stbox::xlen(buf_cip));
  _cipher = ypc::bref(cipher, cipher_size);
  return t;
}

uint32_t keymgr_sgx_module::decrypt_message(const uint8_t *sealed_private_key,
                                            uint32_t sealed_size,
                                            const uint8_t *cipher,
                                            uint32_t cipher_size, bref &_data) {
  uint8_t *data;
  uint32_t data_size;
  stbox::buffer_length_t buf_data(&data_size, &data,
                                  ::get_decrypted_message_size, cipher_size);
  auto t = ecall<uint32_t>(::decrypt_message, (uint8_t *)sealed_private_key,
                           sealed_size, (uint8_t *)cipher, cipher_size,
                           stbox::xmem(buf_data), stbox::xlen(buf_data));

  _data = bref(data, data_size);
  return t;
}

uint32_t keymgr_sgx_module::session_request(sgx_dh_msg1_t *dh_msg1,
                                            uint32_t *session_id) {
  return ecall<uint32_t>(::msession_request, dh_msg1, session_id);
}
uint32_t keymgr_sgx_module::exchange_report(sgx_dh_msg2_t *dh_msg2,
                                            sgx_dh_msg3_t *dh_msg3,
                                            uint32_t session_id) {
  return ecall<uint32_t>(::mexchange_report, dh_msg2, dh_msg3, session_id);
}
uint32_t keymgr_sgx_module::generate_response(secure_message_t *req_message,
                                              size_t req_message_size,
                                              size_t max_payload_size,
                                              secure_message_t *resp_message,
                                              size_t resp_message_size,
                                              uint32_t session_id) {
  return ecall<uint32_t>(::mgenerate_response, req_message, req_message_size,
                         max_payload_size, resp_message, resp_message_size,
                         session_id);
}
uint32_t keymgr_sgx_module::end_session(uint32_t session_id) {
  return ecall<uint32_t>(::mend_session, session_id);
}

uint32_t keymgr_sgx_module::forward_private_key(
    const uint8_t *encrypted_private_key, uint32_t cipher_size,
    const uint8_t *epublic_key, uint32_t epkey_size, const uint8_t *ehash,
    uint32_t ehash_size, const uint8_t *sig, uint32_t sig_size) {
  return ecall<uint32_t>(::forward_private_key, encrypted_private_key,
                         cipher_size, epublic_key, epkey_size, ehash,
                         ehash_size, sig, sig_size);
}

// uint32_t keymgr_sgx_module::forward_extra_data_usage_license(
// const ypc::bytes &enclave_pkey, const ypc::bytes &data_hash,
// const ypc::bytes &data_usage_license) {
// return ecall<uint32_t>(
//::forward_extra_data_usage_license, (uint8_t *)enclave_pkey.data(),
// enclave_pkey.size(), (uint8_t *)data_hash.data(), data_hash.size(),
//(uint8_t *)data_usage_license.data(), data_usage_license.size());
//}

uint32_t
keymgr_sgx_module::set_access_control_policy(const ypc::bytes &policy) {
  return ecall<uint32_t>(::set_access_control_policy, (uint8_t *)policy.data(),
                         policy.size());
}

uint32_t
keymgr_sgx_module::create_report_for_pkey(const sgx_target_info_t *p_qe3_target,
                                          const stbox::bytes &pkey,
                                          sgx_report_t *p_report) {

  return ecall<uint32_t>(::create_report_for_pkey, p_qe3_target, pkey.data(),
                         pkey.size(), p_report);
}
