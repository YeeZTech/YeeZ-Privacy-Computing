#pragma once

/*
 * Build-time support for parser plugins. This API is consumed by plugin DSOs,
 * not by fid_analyzer. It is C-compatible so adapters may stay independent of
 * compiler-specific C++ ABIs.
 */

#include <stdint.h>

#include <sgx_eid.h>
#include <sgx_error.h>
#include <toolkit/analyzer/fid_parser_plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef sgx_status_t (*fid_parser_ecall_noarg_fn)(sgx_enclave_id_t,
                                                   uint32_t *);
typedef sgx_status_t (*fid_parser_ecall_input_fn)(sgx_enclave_id_t,
                                                   uint32_t *,
                                                   const uint8_t *,
                                                   uint32_t);
typedef sgx_status_t (*fid_parser_ecall_output_fn)(sgx_enclave_id_t,
                                                    uint32_t *, uint8_t *,
                                                    uint32_t);

typedef void *(*fid_parser_create_context_fn)(
    const fid_parser_host_callbacks *host_callbacks,
    fid_parser_call_status *status);
typedef void (*fid_parser_destroy_context_fn)(void *instance_context);

/* Internal runtime behavior; these bits are not exposed in plugin api.flags. */
#define FID_PARSER_PLUGIN_RUNTIME_CLEANUP_CREATED_FILES (1u << 31)

typedef struct fid_parser_plugin_runtime_config {
  uint32_t struct_size;
  const char *module_id;
  const char *edl_contract_fingerprint;
  uint32_t flags;

  fid_parser_ecall_noarg_fn begin_parse_data_item;
  fid_parser_ecall_input_fn parse_data_item;
  fid_parser_ecall_noarg_fn end_parse_data_item;
  fid_parser_ecall_noarg_fn get_enclave_hash_size;
  fid_parser_ecall_output_fn get_enclave_hash;
  fid_parser_ecall_noarg_fn get_analyze_result_size;
  fid_parser_ecall_output_fn get_analyze_result;
  fid_parser_ecall_input_fn init_data_source;
  fid_parser_ecall_input_fn init_model;
  fid_parser_ecall_noarg_fn get_parser_type;

  /* Optional per-instance state used by EDL-specific OCALL handlers. */
  fid_parser_create_context_fn create_context;
  fid_parser_destroy_context_fn destroy_context;
} fid_parser_plugin_runtime_config;

/* Called by the single exported query wrapper in each parser plugin. */
int32_t fid_parser_plugin_runtime_query(
    const fid_parser_plugin_runtime_config *config,
    uint32_t host_abi_major, uint32_t host_abi_minor,
    uint32_t host_api_struct_size, const fid_parser_plugin_api **api_out);

/* Valid only while the current thread is servicing a plugin ECALL. */
void *fid_parser_plugin_runtime_active_context(void);
const fid_parser_host_callbacks *
fid_parser_plugin_runtime_active_host_callbacks(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
