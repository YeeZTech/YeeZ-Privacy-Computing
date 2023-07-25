#ifndef EPARSER_T_INTERFACE_H__
#define EPARSER_T_INTERFACE_H__

#include "sgx_edger8r.h" /* for sgx_ocall etc. */
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#include "sgx_eid.h"
#include "ypc/stbox/tsgx/channel/dh_cdef.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

uint32_t begin_parse_data_item(void);
uint32_t parse_data_item(const uint8_t *sealed_data, uint32_t len);
uint32_t end_parse_data_item(void);
uint32_t get_enclave_hash_size(void);
uint32_t get_enclave_hash(uint8_t *hash, uint32_t hash_size);
uint32_t get_analyze_result_size(void);
uint32_t get_analyze_result(uint8_t *analyze_result, uint32_t res_size);
uint32_t init_data_source(const uint8_t *data_source_info, uint32_t data_size);
uint32_t init_model(const uint8_t *model, uint32_t data_size);
uint32_t get_parser_type(void);
uint64_t get_ypc_analyzer_version(void);
uint64_t stbox_common_version(void);

sgx_status_t SGX_CDECL next_data_batch(uint32_t *retval,
                                       const uint8_t *data_hash,
                                       uint32_t hash_size, uint8_t **data,
                                       uint32_t *len);
sgx_status_t SGX_CDECL free_data_batch(uint8_t *data);
sgx_status_t SGX_CDECL download_oram_params_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size, 
                                                  uint32_t *block_num, uint32_t *bucket_num_N, uint8_t *level_num_L, 
                                                  uint32_t *bucket_str_size, uint32_t *batch_str_size);
sgx_status_t SGX_CDECL get_block_id_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size, 
                                          uint32_t *block_id,
                                          const uint8_t *param_hash, uint32_t param_hash_size);
sgx_status_t SGX_CDECL download_position_map_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size, 
                                                   uint8_t ** position_map, uint32_t *len);
sgx_status_t SGX_CDECL update_position_map_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size,
                                                 uint8_t * position_map, uint32_t len);
sgx_status_t SGX_CDECL download_path_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size,
                                           uint32_t leaf, uint8_t ** encrpypted_path, uint32_t *len);                                           
sgx_status_t SGX_CDECL download_stash_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size,
                                            uint8_t ** stash, uint32_t *len);
sgx_status_t SGX_CDECL update_stash_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size,
                                          uint8_t * stash, uint32_t len);
sgx_status_t SGX_CDECL upload_path_OCALL(uint32_t *retval, const uint8_t *data_hash, uint32_t hash_size,
                                         uint32_t leaf, uint8_t * encrpypted_path, uint32_t len);

sgx_status_t SGX_CDECL km_session_request_oram_ocall(uint32_t *retval,
                                                sgx_dh_msg1_t *dh_msg1,
                                                uint32_t *session_id);
sgx_status_t SGX_CDECL km_exchange_report_oram_ocall(uint32_t *retval,
                                                sgx_dh_msg2_t *dh_msg2,
                                                sgx_dh_msg3_t *dh_msg3,
                                                uint32_t session_id);
sgx_status_t SGX_CDECL km_send_request_oram_ocall(
    uint32_t *retval, uint32_t session_id, secure_message_t *req_message,
    size_t req_message_size, size_t max_payload_size,
    secure_message_t *resp_message, size_t resp_message_size);
sgx_status_t SGX_CDECL km_end_session_oram_ocall(uint32_t *retval,
                                            uint32_t session_id);

sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf);
sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int *retval,
                                                             const void *self);
sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int *retval,
                                                            const void *waiter);
sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(
    int *retval, const void *waiter, const void *self);
sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(
    int *retval, const void **waiters, size_t total);
sgx_status_t SGX_CDECL km_session_request_ocall(uint32_t *retval,
                                                sgx_dh_msg1_t *dh_msg1,
                                                uint32_t *session_id);
sgx_status_t SGX_CDECL km_exchange_report_ocall(uint32_t *retval,
                                                sgx_dh_msg2_t *dh_msg2,
                                                sgx_dh_msg3_t *dh_msg3,
                                                uint32_t session_id);
sgx_status_t SGX_CDECL km_send_request_ocall(
    uint32_t *retval, uint32_t session_id, secure_message_t *req_message,
    size_t req_message_size, size_t max_payload_size,
    secure_message_t *resp_message, size_t resp_message_size);
sgx_status_t SGX_CDECL km_end_session_ocall(uint32_t *retval,
                                            uint32_t session_id);
sgx_status_t SGX_CDECL ocall_print_string(const char *buf);
sgx_status_t SGX_CDECL ocall_log_string(uint32_t rank, const char *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
