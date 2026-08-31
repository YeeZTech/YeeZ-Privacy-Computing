#include "dianshu_parser_u.h"
#include "ocall_def.h"

#include <ypc/core/byte.h>
#include <toolkit/analyzer/fid_parser_plugin.h>
#include <toolkit/analyzer/fid_parser_plugin_runtime.h>

#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <vector>

#ifndef FID_PARSER_EDL_CONTRACT_FINGERPRINT
#error "add_fid_parser_plugin must define FID_PARSER_EDL_CONTRACT_FINGERPRINT"
#endif
#ifndef FID_PARSER_MODULE_ID
#error "add_fid_parser_plugin must define FID_PARSER_MODULE_ID"
#endif

extern uint32_t handle_mp4(const ypc::bytes &, ypc::bytes &);
extern uint32_t handle_csv(const ypc::bytes &, ypc::bytes &);
extern uint32_t handle_tsv(const ypc::bytes &, ypc::bytes &);
extern uint32_t handle_pdf(const ypc::bytes &, ypc::bytes &);
extern uint32_t handle_zip(const ypc::bytes &, ypc::bytes &);
extern uint32_t handle_rar(const ypc::bytes &, ypc::bytes &);
extern uint32_t handle_mp4_cut(const ypc::bytes &, ypc::bytes &);

namespace {

struct dianshu_plugin_context {
  std::vector<uint8_t> scratch;
};

void clear_status(fid_parser_call_status *status) noexcept {
  if (status == nullptr) {
    return;
  }
  status->struct_size = sizeof(*status);
  status->plugin_status = FID_PARSER_PLUGIN_OK;
  status->sgx_status = SGX_SUCCESS;
  status->enclave_status = 0;
}

void *create_context(const fid_parser_host_callbacks *,
                     fid_parser_call_status *status) noexcept {
  clear_status(status);
  auto *context = new (std::nothrow) dianshu_plugin_context();
  if (context == nullptr && status != nullptr) {
    status->plugin_status = FID_PARSER_PLUGIN_INTERNAL_ERROR;
  }
  return context;
}

void destroy_context(void *context) noexcept {
  delete static_cast<dianshu_plugin_context *>(context);
}

void log_error(const char *message) noexcept {
  const auto *callbacks = fid_parser_plugin_runtime_active_host_callbacks();
  if (message == nullptr || callbacks == nullptr || callbacks->log == nullptr) {
    return;
  }
  const auto max_size = static_cast<size_t>(std::numeric_limits<uint32_t>::max());
  const size_t message_size = std::strlen(message);
  const uint32_t size = static_cast<uint32_t>(
      message_size > max_size ? max_size : message_size);
  try {
    callbacks->log(callbacks->host_context, 2u, message, size);
  } catch (...) {
    // A host callback must not throw through the EDL trampoline.
  }
}

using handler_fn = uint32_t (*)(const ypc::bytes &, ypc::bytes &);

handler_fn find_handler(int type) noexcept {
  switch (static_cast<ParserType>(type)) {
  case ParserType::PARSER_MP4:
    return handle_mp4;
  case ParserType::PARSER_CSV:
    return handle_csv;
  case ParserType::PARSER_TSV:
    return handle_tsv;
  case ParserType::PARSER_PDF:
    return handle_pdf;
  case ParserType::PARSER_ZIP:
    return handle_zip;
  case ParserType::PARSER_RAR:
    return handle_rar;
  case ParserType::PARSER_MP4_CUT:
    return handle_mp4_cut;
  }
  return nullptr;
}

const fid_parser_plugin_runtime_config runtime_config = {
    sizeof(fid_parser_plugin_runtime_config),
    FID_PARSER_MODULE_ID,
    FID_PARSER_EDL_CONTRACT_FINGERPRINT,
    FID_PARSER_PLUGIN_FLAG_SINGLE_ACTIVE_INSTANCE |
        FID_PARSER_PLUGIN_FLAG_SINGLE_INFLIGHT_ECALL |
        FID_PARSER_PLUGIN_FLAG_NO_SWITCHLESS_OCALL |
        FID_PARSER_PLUGIN_RUNTIME_CLEANUP_CREATED_FILES,
    dianshu_parser_begin_parse_data_item,
    dianshu_parser_parse_data_item,
    dianshu_parser_end_parse_data_item,
    dianshu_parser_get_enclave_hash_size,
    dianshu_parser_get_enclave_hash,
    dianshu_parser_get_analyze_result_size,
    dianshu_parser_get_analyze_result,
    dianshu_parser_init_data_source,
    dianshu_parser_init_model,
    dianshu_parser_get_parser_type,
    create_context,
    destroy_context,
};

} // namespace

extern "C" uint32_t ocall_get_data(uint8_t *input, uint32_t input_size,
                                    uint8_t **output,
                                    uint32_t *output_size) {
  if (output != nullptr) {
    *output = nullptr;
  }
  if (output_size != nullptr) {
    *output_size = 0;
  }
  if (output == nullptr || output_size == nullptr ||
      (input == nullptr && input_size != 0u)) {
    return FID_PARSER_PLUGIN_INVALID_ARGUMENT;
  }

  try {
    auto *context = static_cast<dianshu_plugin_context *>(
        fid_parser_plugin_runtime_active_context());
    if (context == nullptr) {
      return FID_PARSER_PLUGIN_NO_ACTIVE_INSTANCE;
    }

    auto package = ypc::make_package<typename ypc::cast_obj_to_package<
        ocall_data_item_t>::type>::from_bytes(ypc::bytes(input, input_size));
    const int type = package.get<::parser_type>();
    const auto data = package.get<::ocall_data>();
    const handler_fn handler = find_handler(type);
    if (handler == nullptr) {
      log_error("ocall_get_data: unknown parser type");
      return FID_PARSER_PLUGIN_INVALID_ARGUMENT;
    }

    ypc::bytes result;
    const uint32_t handler_status = handler(data, result);
    if (handler_status != 0u) {
      log_error("ocall_get_data: handler returned failure");
      return handler_status;
    }
    if (result.empty()) {
      log_error("ocall_get_data: handler returned an empty result");
      return FID_PARSER_PLUGIN_INTERNAL_ERROR;
    }
    if (result.size() > std::numeric_limits<uint32_t>::max()) {
      log_error("ocall_get_data: handler output exceeds the EDL limit");
      return FID_PARSER_PLUGIN_INTERNAL_ERROR;
    }

    context->scratch.resize(result.size());
    if (!result.empty()) {
      std::memcpy(context->scratch.data(), result.data(), result.size());
      *output = context->scratch.data();
    }
    *output_size = static_cast<uint32_t>(context->scratch.size());
    return 0u;
  } catch (const std::exception &error) {
    log_error(error.what());
  } catch (...) {
    log_error("ocall_get_data: unknown exception");
  }
  *output = nullptr;
  *output_size = 0;
  return FID_PARSER_PLUGIN_EXCEPTION;
}

extern "C" FID_PARSER_PLUGIN_EXPORT int32_t fid_parser_plugin_query(
    uint32_t host_abi_major, uint32_t host_abi_minor,
    uint32_t host_api_struct_size,
    const fid_parser_plugin_api **api_out) {
  if (api_out != nullptr) {
    *api_out = nullptr;
  }
  try {
    return fid_parser_plugin_runtime_query(
        &runtime_config, host_abi_major, host_abi_minor,
        host_api_struct_size, api_out);
  } catch (...) {
    return FID_PARSER_PLUGIN_EXCEPTION;
  }
}
