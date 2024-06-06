#include "sgx_bridge.h"
#include "ypc/core/memref.h"

using stx_status = stbox::stx_status;
std::shared_ptr<parser> g_parser;
std::shared_ptr<oram_parser> o_parser;

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



uint32_t write_convert_data_structure(int64_t filepos, uint8_t * convert_data_bytes, uint32_t len);
uint32_t download_convert_params_ocall(uint32_t *block_num, long int *oram_tree_filepos, 
    uint64_t *item_num_each_batch, uint64_t *item_size);

}

uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1,
                                  uint32_t *session_id) {
  return g_parser->keymgr()->session_request(dh_msg1, session_id);
}
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id) {
  return g_parser->keymgr()->exchange_report(dh_msg2, dh_msg3, session_id);
}
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size) {
  return g_parser->keymgr()->generate_response(req_message, req_message_size,
                                               max_payload_size, resp_message,
                                               resp_message_size, session_id);
}
uint32_t km_end_session_ocall(uint32_t session_id) {
  return g_parser->keymgr()->end_session(session_id);
}

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len) {
  return g_parser->next_data_batch(data_hash, hash_size, data, len);
}
void free_data_batch(uint8_t *data) { g_parser->free_data_batch(data); }



uint32_t km_session_request_oram_ocall(sgx_dh_msg1_t *dh_msg1,
                                  uint32_t *session_id) {
  return o_parser->keymgr()->session_request(dh_msg1, session_id);
}
uint32_t km_exchange_report_oram_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id) {
  return o_parser->keymgr()->exchange_report(dh_msg2, dh_msg3, session_id);
}
uint32_t km_send_request_oram_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size) {
  return o_parser->keymgr()->generate_response(req_message, req_message_size,
                                               max_payload_size, resp_message,
                                               resp_message_size, session_id);
}
uint32_t km_end_session_oram_ocall(uint32_t session_id) {
  return o_parser->keymgr()->end_session(session_id);
}

uint32_t download_oram_params_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                        uint32_t *block_num, uint32_t *bucket_num_N, uint8_t *level_num_L, 
                        uint32_t *bucket_str_size, uint32_t *batch_str_size) {
  return o_parser->download_oram_params_OCALL(data_hash, hash_size, block_num, 
                  bucket_num_N, level_num_L, bucket_str_size, batch_str_size);
}

uint32_t get_block_id_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                            uint32_t *block_id,
                            const uint8_t *param_hash, uint32_t param_hash_size) {
  return o_parser->get_block_id_OCALL(data_hash, hash_size, block_id, param_hash, param_hash_size);
}

uint32_t download_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                     uint8_t ** position_map, uint32_t *len) {
  return o_parser->download_position_map_OCALL(data_hash, hash_size, position_map, len);
}

uint32_t update_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                   uint8_t * position_map, uint32_t len) {
  return o_parser->update_position_map_OCALL(data_hash, hash_size, position_map, len);
}

uint32_t download_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                             uint32_t leaf, uint8_t ** encrpypted_path, uint32_t *len) {
  return o_parser->download_path_OCALL(data_hash, hash_size, leaf, encrpypted_path, len);
}

uint32_t download_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                              uint8_t ** stash, uint32_t *len) {
  return o_parser->download_stash_OCALL(data_hash, hash_size, stash, len);
}

uint32_t update_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                            uint8_t * stash, uint32_t len) {
  return o_parser->update_stash_OCALL(data_hash, hash_size, stash, len);
}

uint32_t upload_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                           uint32_t leaf, uint8_t * encrpypted_path, uint32_t len) {
  return o_parser->upload_path_OCALL(data_hash, hash_size, leaf, encrpypted_path, len);
}

uint32_t download_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                    uint32_t leaf, uint8_t ** merkle_hash, uint32_t *len) {
  return o_parser->download_merkle_hash_OCALL(data_hash, hash_size, leaf, merkle_hash, len);
}

uint32_t update_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                  uint32_t leaf, uint8_t * merkle_hash, uint32_t len) {
  return o_parser->update_merkle_hash_OCALL(data_hash, hash_size, leaf, merkle_hash, len);
}



uint32_t write_convert_data_structure(int64_t filepos, uint8_t * convert_data_bytes, uint32_t len) {
  return g_parser->write_convert_data_structure(filepos, convert_data_bytes, len);
}

uint32_t download_convert_params_ocall(uint32_t *block_num, long int *oram_tree_filepos, 
    uint64_t *item_num_each_batch, uint64_t *item_size) {
  return g_parser->download_convert_params_ocall(block_num, oram_tree_filepos, item_num_each_batch, item_size);
}