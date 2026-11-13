#pragma once

/*
 * Stable host/plugin boundary for Fidelius parser modules.
 *
 * This header is deliberately C-only. Memory is always owned by the caller,
 * strings in the descriptor have static storage in the plugin, and no C++
 * object or exception may cross this boundary.
 */

#include <stddef.h>
#include <stdint.h>

#include <sgx_dh.h>
#include <sgx_error.h>
#include <ypc/stbox/tsgx/channel/dh_cdef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FID_PARSER_PLUGIN_ABI_MAJOR 1u
#define FID_PARSER_PLUGIN_ABI_MINOR 0u
#define FID_PARSER_PLUGIN_QUERY_SYMBOL "fid_parser_plugin_query"

/*
 * Minor revisions within one ABI major are append-only. Hosts and plugins use
 * struct_size to negotiate the common prefix and must not require exact minor
 * equality.
 */

#if defined(__GNUC__) || defined(__clang__)
#define FID_PARSER_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define FID_PARSER_PLUGIN_EXPORT
#endif

typedef struct fid_parser_plugin_instance fid_parser_plugin_instance;

typedef enum fid_parser_plugin_status {
  FID_PARSER_PLUGIN_OK = 0,
  FID_PARSER_PLUGIN_INVALID_ARGUMENT = 1,
  FID_PARSER_PLUGIN_ABI_MISMATCH = 2,
  FID_PARSER_PLUGIN_BUSY = 3,
  FID_PARSER_PLUGIN_NO_ACTIVE_INSTANCE = 4,
  FID_PARSER_PLUGIN_BUFFER_TOO_SMALL = 5,
  FID_PARSER_PLUGIN_EXCEPTION = 6,
  FID_PARSER_PLUGIN_INTERNAL_ERROR = 7
} fid_parser_plugin_status;

/*
 * enclave_status is meaningful only when plugin_status is OK and sgx_status
 * is SGX_SUCCESS. Every plugin entry point resets all fields before work.
 */
typedef struct fid_parser_call_status {
  uint32_t struct_size;
  int32_t plugin_status;
  uint32_t sgx_status;
  uint32_t enclave_status;
} fid_parser_call_status;

typedef uint32_t (*fid_parser_next_data_batch_fn)(
    void *host_context, const uint8_t *data_hash, uint32_t hash_size,
    uint8_t **data, uint32_t *len);

typedef uint32_t (*fid_parser_km_session_request_fn)(
    void *host_context, sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);

typedef uint32_t (*fid_parser_km_exchange_report_fn)(
    void *host_context, sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
    uint32_t session_id);

typedef uint32_t (*fid_parser_km_send_request_fn)(
    void *host_context, uint32_t session_id, secure_message_t *req_message,
    uint64_t req_message_size, uint64_t max_payload_size,
    secure_message_t *resp_message, uint64_t resp_message_size);

typedef uint32_t (*fid_parser_km_end_session_fn)(void *host_context,
                                                 uint32_t session_id);

typedef void (*fid_parser_log_fn)(void *host_context, uint32_t rank,
                                  const char *message,
                                  uint32_t message_size);

typedef struct fid_parser_host_callbacks {
  uint32_t struct_size;
  void *host_context;
  fid_parser_next_data_batch_fn next_data_batch;
  fid_parser_km_session_request_fn km_session_request;
  fid_parser_km_exchange_report_fn km_exchange_report;
  fid_parser_km_send_request_fn km_send_request;
  fid_parser_km_end_session_fn km_end_session;
  fid_parser_log_fn log;
} fid_parser_host_callbacks;

enum fid_parser_plugin_flags {
  FID_PARSER_PLUGIN_FLAG_SINGLE_ACTIVE_INSTANCE = 1u << 0,
  FID_PARSER_PLUGIN_FLAG_SINGLE_INFLIGHT_ECALL = 1u << 1,
  FID_PARSER_PLUGIN_FLAG_NO_SWITCHLESS_OCALL = 1u << 2
};

typedef struct fid_parser_plugin_api {
  uint32_t struct_size;
  uint32_t abi_major;
  uint32_t abi_minor;
  const char *module_id;
  const char *edl_contract_fingerprint;
  uint32_t flags;

  void (*create)(const char *signed_enclave_path,
                 const fid_parser_host_callbacks *host_callbacks,
                 fid_parser_plugin_instance **instance_out,
                 fid_parser_call_status *status);
  void (*destroy)(fid_parser_plugin_instance *instance,
                  fid_parser_call_status *status);

  void (*begin_parse_data_item)(fid_parser_plugin_instance *instance,
                                fid_parser_call_status *status);
  void (*parse_data_item)(fid_parser_plugin_instance *instance,
                          const uint8_t *data, uint32_t data_size,
                          fid_parser_call_status *status);
  void (*end_parse_data_item)(fid_parser_plugin_instance *instance,
                              fid_parser_call_status *status);

  void (*init_data_source)(fid_parser_plugin_instance *instance,
                           const uint8_t *data, uint32_t data_size,
                           fid_parser_call_status *status);
  void (*init_model)(fid_parser_plugin_instance *instance,
                     const uint8_t *data, uint32_t data_size,
                     fid_parser_call_status *status);
  void (*get_parser_type)(fid_parser_plugin_instance *instance,
                          uint32_t *parser_type_out,
                          fid_parser_call_status *status);

  /*
   * A NULL buffer with capacity zero performs a length query. required_size
   * is always cleared first. For a nonempty result the length query, or any
   * insufficient capacity, returns BUFFER_TOO_SMALL while required_size holds
   * the required byte count.
   */
  void (*get_enclave_hash)(fid_parser_plugin_instance *instance,
                           uint8_t *buffer, uint32_t capacity,
                           uint32_t *required_size,
                           fid_parser_call_status *status);
  void (*get_analyze_result)(fid_parser_plugin_instance *instance,
                             uint8_t *buffer, uint32_t capacity,
                             uint32_t *required_size,
                             fid_parser_call_status *status);
} fid_parser_plugin_api;

#define FID_PARSER_PLUGIN_API_V1_0_SIZE                                  \
  ((uint32_t)(offsetof(fid_parser_plugin_api, get_analyze_result) +       \
              sizeof(((fid_parser_plugin_api *)0)->get_analyze_result)))

/*
 * The query function is the only public ELF symbol in a parser plugin.
 * api_out points to immutable storage owned by the DSO and remains valid while
 * the DSO is loaded.
 */
typedef int32_t (*fid_parser_plugin_query_fn)(
    uint32_t host_abi_major, uint32_t host_abi_minor,
    uint32_t host_api_struct_size, const fid_parser_plugin_api **api_out);

FID_PARSER_PLUGIN_EXPORT int32_t fid_parser_plugin_query(
    uint32_t host_abi_major, uint32_t host_abi_minor,
    uint32_t host_api_struct_size, const fid_parser_plugin_api **api_out);

#ifdef __cplusplus
} /* extern "C" */
#endif
