#include "stbox/tsgx/channel/dh_session.h"

namespace stbox {

dh_session::dh_session()
    : m_send_request_ocall_set(false), m_verify_peer_enclave_trust_set(false) {}

dh_session::~dh_session() {}

stx_status dh_session::send_request_recv_response(const char *inp_buff,
                                                  size_t inp_buff_len,
                                                  size_t max_out_buff_size,
                                                  bytes &out_buff) {
  if (!m_send_request_ocall_set || !m_verify_peer_enclave_trust_set) {
    return stx_status::invalid_parameter_error;
  }

  const uint8_t *plaintext;
  uint32_t plaintext_length;
  sgx_status_t status;
  uint32_t retstatus;
  secure_message_t *req_message = nullptr;
  secure_message_t *resp_message = nullptr;
  uint32_t decrypted_data_length;
  uint32_t plain_text_offset;
  uint8_t l_tag[TAG_SIZE];
  size_t max_resp_message_length;
  plaintext = (const uint8_t *)(" ");
  plaintext_length = 0;
  scope_guard _req([&req_message]() {
    if (req_message) {
      free(req_message);
    }
    req_message = nullptr;
  });
  scope_guard _res([&resp_message]() {
    if (resp_message) {
      free(resp_message);
    }
    resp_message = nullptr;
  });

  // TODO: we may need to check the status, or state here
  // if (!session_info || !inp_buff) {
  // return INVALID_PARAMETER_ERROR;
  //}
  // Check if the nonce for the session has not exceeded 2^32-2 if so end
  // session and start a new session
  // if (session_info->active.counter == ((uint32_t)-2)) {
  // close_session(session_info);
  // create_session(session_info);
  //}

  // Allocate memory for the AES-GCM request message
  req_message =
      (secure_message_t *)malloc(sizeof(secure_message_t) + inp_buff_len);
  if (!req_message)
    return stx_status::malloc_error;
  memset(req_message, 0, sizeof(secure_message_t) + inp_buff_len);

  const uint32_t data2encrypt_length = (uint32_t)inp_buff_len;

  // Set the payload size to data to encrypt length
  req_message->message_aes_gcm_data.payload_size = data2encrypt_length;

  // Use the session nonce as the payload IV
  memcpy(req_message->message_aes_gcm_data.reserved, &m_state.active.counter,
         sizeof(m_state.active.counter));

  // Set the session ID of the message to the current session id
  req_message->session_id = m_session_id;

  // Prepare the request message with the encrypted payload
  status = sgx_rijndael128GCM_encrypt(
      &m_state.active.AEK, (uint8_t *)inp_buff, data2encrypt_length,
      reinterpret_cast<uint8_t *>(&(req_message->message_aes_gcm_data.payload)),
      reinterpret_cast<uint8_t *>(
          &(req_message->message_aes_gcm_data.reserved)),
      sizeof(req_message->message_aes_gcm_data.reserved), plaintext,
      plaintext_length, &(req_message->message_aes_gcm_data.payload_tag));

  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }

  // Allocate memory for the response message
  resp_message =
      (secure_message_t *)malloc(sizeof(secure_message_t) + max_out_buff_size);
  if (!resp_message) {
    return stx_status::malloc_error;
  }

  memset(resp_message, 0, sizeof(secure_message_t) + max_out_buff_size);

  // Ocall to send the request to the Destination Enclave and get the response
  // message back
  retstatus = m_send_request_ocall(
      m_session_id, req_message, (sizeof(secure_message_t) + inp_buff_len),
      max_out_buff_size, resp_message,
      (sizeof(secure_message_t) + max_out_buff_size));
  if (static_cast<stx_status>(retstatus) != stx_status::success) {
    return static_cast<stx_status>(retstatus);
  }

  max_resp_message_length = sizeof(secure_message_t) + max_out_buff_size;

  if (sizeof(resp_message) > max_resp_message_length) {
    return stx_status::invalid_parameter_error;
  }

  // Code to process the response message from the Destination Enclave

  decrypted_data_length = resp_message->message_aes_gcm_data.payload_size;
  plain_text_offset = decrypted_data_length;
  out_buff = bytes(decrypted_data_length);

  memset(&l_tag, 0, TAG_SIZE);


  // Decrypt the response message payload
  status = sgx_rijndael128GCM_decrypt(
      &m_state.active.AEK, resp_message->message_aes_gcm_data.payload,
      out_buff.size(), out_buff.data(),
      reinterpret_cast<uint8_t *>(
          &(resp_message->message_aes_gcm_data.reserved)),
      sizeof(resp_message->message_aes_gcm_data.reserved),
      &(resp_message->message_aes_gcm_data.payload[plain_text_offset]),
      plaintext_length, &resp_message->message_aes_gcm_data.payload_tag);

  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }

  // Verify if the nonce obtained in the response is equal to the session nonce
  // + 1 (Prevents replay attacks)
  if (*(uint32_t *)(resp_message->message_aes_gcm_data.reserved) !=
      (m_state.active.counter + 1)) {
    return stx_status::invalid_parameter_error;
  }

  // Update the value of the session nonce in the source enclave
  m_state.active.counter = m_state.active.counter + 1;
  return stx_status::success;
}

stx_status dh_session::generate_response(secure_message_t *req_message,
                                         size_t req_message_size,
                                         const handler_func_t &handler,
                                         size_t max_payload_size,
                                         secure_message_t *resp_message,
                                         size_t resp_message_size) {

  const uint8_t *plaintext;
  uint32_t plaintext_length;
  uint8_t *decrypted_data = nullptr;
  uint32_t decrypted_data_length;
  uint32_t plain_text_offset;
  size_t resp_data_length;
  size_t resp_message_calc_size;
  char *resp_data = nullptr;
  uint8_t l_tag[TAG_SIZE];
  size_t header_size, expected_payload_size;
  secure_message_t *temp_resp_message = nullptr;
  uint32_t ret;
  sgx_status_t status;

  plaintext = (const uint8_t *)(" ");
  plaintext_length = 0;

  if (!req_message || !resp_message) {
    return stx_status::invalid_parameter_error;
  }

  if (m_status != session_status::active) {
    return stx_status::invalid_session;
  }

  // Set the decrypted data length to the payload size obtained from the message
  decrypted_data_length = req_message->message_aes_gcm_data.payload_size;

  header_size = sizeof(secure_message_t);
  expected_payload_size = req_message_size - header_size;

  // Verify the size of the payload
  if (expected_payload_size != decrypted_data_length) {
    return stx_status::invalid_parameter_error;
  }

  memset(&l_tag, 0, TAG_SIZE);
  plain_text_offset = decrypted_data_length;
  decrypted_data = (uint8_t *)malloc(decrypted_data_length);
  if (!decrypted_data) {
    return stx_status::malloc_error;
  }
  malloc_memory_guard<uint8_t> _dd(decrypted_data);

  memset(decrypted_data, 0, decrypted_data_length);

  // Decrypt the request message payload from source enclave
  status = sgx_rijndael128GCM_decrypt(
      &m_state.active.AEK, req_message->message_aes_gcm_data.payload,
      decrypted_data_length, decrypted_data,
      reinterpret_cast<uint8_t *>(
          &(req_message->message_aes_gcm_data.reserved)),
      sizeof(req_message->message_aes_gcm_data.reserved),
      &(req_message->message_aes_gcm_data.payload[plain_text_offset]),
      plaintext_length, &req_message->message_aes_gcm_data.payload_tag);

  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }
  bytes resp_str = handler(decrypted_data, decrypted_data_length, this);

  resp_data_length = resp_str.size();
  if (resp_str.size() > max_payload_size) {
    return stx_status::out_buffer_length_error;
  }

  resp_message_calc_size = sizeof(secure_message_t) + resp_data_length;

  if (resp_message_calc_size > resp_message_size) {
    return stx_status::out_buffer_length_error;
  }

  // Code to build the response back to the Source Enclave
  temp_resp_message = (secure_message_t *)malloc(resp_message_calc_size);
  if (!temp_resp_message) {
    return stx_status::malloc_error;
  }
  malloc_memory_guard<secure_message_t> _t(temp_resp_message);

  memset(temp_resp_message, 0, sizeof(secure_message_t) + resp_data_length);
  const uint32_t data2encrypt_length = (uint32_t)resp_data_length;
  temp_resp_message->session_id = m_session_id;
  temp_resp_message->message_aes_gcm_data.payload_size = data2encrypt_length;

  // Increment the Session Nonce (Replay Protection)
  m_state.active.counter = m_state.active.counter + 1;

  // Set the response nonce as the session nonce
  memcpy(&temp_resp_message->message_aes_gcm_data.reserved,
         &m_state.active.counter, sizeof(m_state.active.counter));

  // Prepare the response message with the encrypted payload
  status = sgx_rijndael128GCM_encrypt(
      &m_state.active.AEK, (uint8_t *)&resp_str[0], data2encrypt_length,
      reinterpret_cast<uint8_t *>(
          &(temp_resp_message->message_aes_gcm_data.payload)),
      reinterpret_cast<uint8_t *>(
          &(temp_resp_message->message_aes_gcm_data.reserved)),
      sizeof(temp_resp_message->message_aes_gcm_data.reserved), plaintext,
      plaintext_length, &(temp_resp_message->message_aes_gcm_data.payload_tag));

  if (SGX_SUCCESS != status) {
    return static_cast<stx_status>(status);
  }

  memset(resp_message, 0, sizeof(secure_message_t) + resp_data_length);
  memcpy(resp_message, temp_resp_message,
         sizeof(secure_message_t) + resp_data_length);

  return stx_status::success;
}
} // namespace stbox
