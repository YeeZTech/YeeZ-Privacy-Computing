#include "ypc/sgx/datahub_sgx_module.h"
#include "enclave_u.h"
#include "sgx_urts.h"
#include <iostream>
#include <stdexcept>

namespace ypc {
datahub_sgx_module::datahub_sgx_module(const char *mod_path)
    : ::stbox::sgx_module(mod_path) {}
datahub_sgx_module::~datahub_sgx_module() {}

uint32_t datahub_sgx_module::get_sealed_data_size(uint32_t encrypt_data_size) {
  uint32_t sealed_data_size =
      ecall<uint32_t>(::get_sealed_data_size, encrypt_data_size);

  return sealed_data_size;
}
bytes datahub_sgx_module::seal_data(const bytes &_data) {
  uint32_t sealed_data_size = get_sealed_data_size(_data.size());
  uint8_t *tmp_sealed_buf = (uint8_t *)malloc(sealed_data_size);
  sgx_status_t retval =
      ecall<sgx_status_t>(::seal_file_data, (uint8_t *)_data.data(),
                          _data.size(), tmp_sealed_buf, sealed_data_size);
  if (retval != SGX_SUCCESS) {
    free(tmp_sealed_buf);
    throw std::runtime_error(std::to_string(retval));
  }
  bytes s(tmp_sealed_buf, sealed_data_size);
  free(tmp_sealed_buf);
  return s;
}

uint32_t datahub_sgx_module::session_request(sgx_dh_msg1_t *dh_msg1,
                                             uint32_t *session_id) {
  return ecall<uint32_t>(::session_request, dh_msg1, session_id);
}
uint32_t datahub_sgx_module::exchange_report(sgx_dh_msg2_t *dh_msg2,
                                             sgx_dh_msg3_t *dh_msg3,
                                             uint32_t session_id) {
  return ecall<uint32_t>(::exchange_report, dh_msg2, dh_msg3, session_id);
}
uint32_t datahub_sgx_module::generate_response(secure_message_t *req_message,
                                               size_t req_message_size,
                                               size_t max_payload_size,
                                               secure_message_t *resp_message,
                                               size_t resp_message_size,
                                               uint32_t session_id) {
  return ecall<uint32_t>(::generate_response, req_message, req_message_size,
                         max_payload_size, resp_message, resp_message_size,
                         session_id);
}
uint32_t datahub_sgx_module::end_session(uint32_t session_id) {
  return ecall<uint32_t>(::end_session, session_id);
}

uint32_t datahub_sgx_module::begin_encrypt_sealed_data() {
  return ecall<uint32_t>(::begin_encrypt_sealed_data);
}
uint32_t datahub_sgx_module::end_encrypt_sealed_data() {
  return ecall<uint32_t>(::end_encrypt_sealed_data);
}

uint32_t
datahub_sgx_module::get_encrypted_sealed_data_size(const uint8_t *sealed_data,
                                                   uint32_t sealed_size) {
  return ecall<uint32_t>(::get_encrypted_sealed_data_size,
                         (uint8_t *)sealed_data, sealed_size);
}

uint32_t datahub_sgx_module::encrypt_sealed_data(const uint8_t *sealed_data,
                                                 uint32_t in_size,
                                                 uint8_t *encrypted_data,
                                                 uint32_t encrypted_size) {
  return ecall<uint32_t>(::encrypt_sealed_data, (uint8_t *)sealed_data, in_size,
                         encrypted_data, encrypted_size);
}
uint32_t datahub_sgx_module::get_encrypted_data_credential(bytes &credential) {
  credential = bytes(ecall<uint32_t>(::get_encrypted_data_credential_size));
  return ecall<uint32_t>(::get_encrypted_data_credential,
                         (uint8_t *)credential.data(), credential.size());
}

uint32_t datahub_sgx_module::generate_data_usage_license(
    const bytes &credential, const bytes &encrypt_param,
    const bytes &enclave_hash, const bytes &pkey4v, const bytes &tee_pkey,
    bytes &license) {
  license = bytes(ecall<uint32_t>(::get_data_usage_license_size));
  return ecall<uint32_t>(
      ::generate_data_usage_license, (uint8_t *)credential.data(),
      credential.size(), (uint8_t *)encrypt_param.data(), encrypt_param.size(),
      (uint8_t *)enclave_hash.data(), enclave_hash.size(),
      (uint8_t *)pkey4v.data(), pkey4v.size(), (uint8_t *)tee_pkey.data(),
      tee_pkey.size(), (uint8_t *)license.data(), license.size());
}
} // namespace ypc

extern "C" {
uint32_t next_sealed_item_data(uint8_t **data, uint32_t *len);
void free_sealed_item_data(uint8_t *data);
}
