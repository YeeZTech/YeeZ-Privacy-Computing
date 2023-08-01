#include "ypc/core/sgx/parser_sgx_module.h"
#include "ypc/stbox/tsgx/channel/dh_cdef.h"
#include <sgx_dh.h>
#include <sgx_urts.h>

using stx_status = stbox::stx_status;

extern "C" {

uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id);
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size);
uint32_t km_end_session_ocall(uint32_t session_id);

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len);
void free_data_batch(uint8_t *data);


uint32_t km_session_request_oram_ocall(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);
uint32_t km_exchange_report_oram_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id);
uint32_t km_send_request_oram_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size);
uint32_t km_end_session_oram_ocall(uint32_t session_id);

uint32_t download_oram_params_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                        uint32_t *block_num, uint32_t *bucket_num_N, uint8_t *level_num_L, 
                        uint32_t *bucket_str_size, uint32_t *batch_str_size);
uint32_t get_block_id_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                            uint32_t *block_id,
                            const uint8_t *param_hash, uint32_t param_hash_size);
uint32_t download_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                     uint8_t ** position_map, uint32_t *len);
uint32_t update_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                   uint8_t * position_map, uint32_t len);
uint32_t download_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                             uint32_t leaf, uint8_t ** encrpypted_path, uint32_t *len);
uint32_t download_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                              uint8_t ** stash, uint32_t *len);
uint32_t update_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                            uint8_t * stash, uint32_t len);
uint32_t upload_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                           uint32_t leaf, uint8_t * encrpypted_path, uint32_t len);
uint32_t download_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                    uint32_t leaf, uint8_t ** merkle_hash, uint32_t *len);
uint32_t update_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                  uint32_t leaf, uint8_t * merkle_hash, uint32_t len);
}


uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1,
                                  uint32_t *session_id) {
  return 0;
}
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id) {
  return 0;
}
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size) {
  return 0;
}
uint32_t km_end_session_ocall(uint32_t session_id) { return 0; }

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len) {
  return 0;
}
void free_data_batch(uint8_t *data) { return; }



uint32_t km_session_request_oram_ocall(sgx_dh_msg1_t *dh_msg1,
                                  uint32_t *session_id) {
  return 0;
}
uint32_t km_exchange_report_oram_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id) {
  return 0;
}
uint32_t km_send_request_oram_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size) {
  return 0;
}
uint32_t km_end_session_oram_ocall(uint32_t session_id) { return 0; }

uint32_t download_oram_params_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                        uint32_t *block_num, uint32_t *bucket_num_N, uint8_t *level_num_L, 
                        uint32_t *bucket_str_size, uint32_t *batch_str_size) {
  return 0;
}

uint32_t get_block_id_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                            uint32_t *block_id,
                            const uint8_t *param_hash, uint32_t param_hash_size) {
  return 0;
}

uint32_t download_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                     uint8_t ** position_map, uint32_t *len) {
  return 0;
}

uint32_t update_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                   uint8_t * position_map, uint32_t len) {
  return 0;
}

uint32_t download_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                             uint32_t leaf, uint8_t ** encrpypted_path, uint32_t *len) {
  return 0;
}

uint32_t download_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                              uint8_t ** stash, uint32_t *len) {
  return 0;
}

uint32_t update_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                            uint8_t * stash, uint32_t len) {
  return 0;
}

uint32_t upload_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                           uint32_t leaf, uint8_t * encrpypted_path, uint32_t len) {
  return 0;
}

uint32_t download_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                    uint32_t leaf, uint8_t ** merkle_hash, uint32_t *len) {
  return 0;
}

uint32_t update_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                  uint32_t leaf, uint8_t * merkle_hash, uint32_t len) {
  return 0;
}