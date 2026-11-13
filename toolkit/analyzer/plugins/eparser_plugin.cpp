#include <toolkit/analyzer/fid_parser_plugin_runtime.h>

#include "eparser_u.h"

#ifndef FID_PARSER_MODULE_ID
#error "FID_PARSER_MODULE_ID must be supplied by add_fid_parser_plugin"
#endif

#ifndef FID_PARSER_EDL_CONTRACT_FINGERPRINT
#error "FID_PARSER_EDL_CONTRACT_FINGERPRINT must be supplied by add_fid_parser_plugin"
#endif

namespace {

const fid_parser_plugin_runtime_config kRuntimeConfig = {
    sizeof(fid_parser_plugin_runtime_config),
    FID_PARSER_MODULE_ID,
    FID_PARSER_EDL_CONTRACT_FINGERPRINT,
    0,
    eparser_begin_parse_data_item,
    eparser_parse_data_item,
    eparser_end_parse_data_item,
    eparser_get_enclave_hash_size,
    eparser_get_enclave_hash,
    eparser_get_analyze_result_size,
    eparser_get_analyze_result,
    eparser_init_data_source,
    eparser_init_model,
    eparser_get_parser_type,
    NULL,
    NULL};

} // namespace

extern "C" FID_PARSER_PLUGIN_EXPORT int32_t fid_parser_plugin_query(
    uint32_t host_abi_major, uint32_t host_abi_minor,
    uint32_t host_api_struct_size, const fid_parser_plugin_api **api_out) {
  try {
    return fid_parser_plugin_runtime_query(
        &kRuntimeConfig, host_abi_major, host_abi_minor,
        host_api_struct_size, api_out);
  } catch (...) {
    if (api_out != NULL) {
      *api_out = NULL;
    }
    return FID_PARSER_PLUGIN_EXCEPTION;
  }
}
