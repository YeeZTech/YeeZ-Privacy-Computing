#include <toolkit/analyzer/fid_parser_plugin_runtime.h>

#include <ypc/core_t/util/file_openmode.h>
#include <ypc/core_t/util/fpos.h>

#include <sgx_urts.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <unistd.h>
#ifndef FID_PARSER_ENCLAVE_DEBUG_FLAG
#define FID_PARSER_ENCLAVE_DEBUG_FLAG SGX_DEBUG_FLAG
#endif

namespace {

const uint32_t kHostCallbackError = UINT32_MAX;

struct KeyHash {
  std::size_t operator()(const std::array<uint8_t, 32> &key) const {
    std::size_t value = 1469598103934665603ULL;
    for (std::size_t i = 0; i < key.size(); ++i) {
      value ^= key[i];
      value *= 1099511628211ULL;
    }
    return value;
  }
};

} // namespace

struct fid_parser_plugin_instance {
  sgx_enclave_id_t enclave_id;
  fid_parser_host_callbacks callbacks;
  void *instance_context;
  std::mutex inflight;
  bool destroying;
  uint32_t next_stream_id;
  std::unordered_map<uint32_t, std::unique_ptr<std::fstream> > files;
  std::unordered_set<std::string> created_files;
  std::unordered_map<std::array<uint8_t, 32>, std::vector<uint8_t>, KeyHash>
      storage;

  fid_parser_plugin_instance()
      : enclave_id(0), callbacks(), instance_context(NULL), destroying(false),
        next_stream_id(1), files(), created_files(), storage() {}
};

namespace {

std::mutex g_runtime_mutex;
const fid_parser_plugin_runtime_config *g_config = NULL;
fid_parser_plugin_instance *g_live_instance = NULL;
fid_parser_plugin_api g_api = {};
thread_local fid_parser_plugin_instance *g_active_instance = NULL;

bool reset_status(fid_parser_call_status *status) {
  if (status == NULL || status->struct_size < sizeof(*status)) {
    return false;
  }
  status->struct_size = sizeof(*status);
  status->plugin_status = FID_PARSER_PLUGIN_OK;
  status->sgx_status = SGX_SUCCESS;
  status->enclave_status = 0;
  return true;
}

void set_plugin_error(fid_parser_call_status *status,
                      fid_parser_plugin_status error) {
  if (status != NULL && status->struct_size >= sizeof(*status)) {
    status->plugin_status = error;
  }
}

bool config_is_valid(const fid_parser_plugin_runtime_config *config) {
  return config != NULL && config->struct_size >= sizeof(*config) &&
         config->module_id != NULL && config->module_id[0] != '\0' &&
         config->edl_contract_fingerprint != NULL &&
         config->edl_contract_fingerprint[0] != '\0' &&
         config->begin_parse_data_item != NULL &&
         config->parse_data_item != NULL &&
         config->end_parse_data_item != NULL &&
         config->get_enclave_hash_size != NULL &&
         config->get_enclave_hash != NULL &&
         config->get_analyze_result_size != NULL &&
         config->get_analyze_result != NULL &&
         config->init_data_source != NULL && config->init_model != NULL &&
         config->get_parser_type != NULL;
}

bool callbacks_are_valid(const fid_parser_host_callbacks *callbacks) {
  return callbacks != NULL && callbacks->struct_size >= sizeof(*callbacks) &&
         callbacks->next_data_batch != NULL &&
         callbacks->km_session_request != NULL &&
         callbacks->km_exchange_report != NULL &&
         callbacks->km_send_request != NULL &&
         callbacks->km_end_session != NULL;
}

class EcallScope {
public:
  EcallScope(fid_parser_plugin_instance *instance,
             fid_parser_call_status *status)
      : instance_(instance), lock_(), entered_(false) {
    if (instance_ == NULL) {
      set_plugin_error(status, FID_PARSER_PLUGIN_INVALID_ARGUMENT);
      return;
    }
    lock_ = std::unique_lock<std::mutex>(instance_->inflight,
                                         std::try_to_lock);
    if (!lock_.owns_lock() || g_active_instance != NULL) {
      set_plugin_error(status, FID_PARSER_PLUGIN_BUSY);
      return;
    }
    {
      std::lock_guard<std::mutex> guard(g_runtime_mutex);
      if (g_live_instance != instance_ || instance_->destroying) {
        set_plugin_error(status, FID_PARSER_PLUGIN_NO_ACTIVE_INSTANCE);
        return;
      }
    }
    g_active_instance = instance_;
    entered_ = true;
  }

  ~EcallScope() {
    if (entered_) {
      g_active_instance = NULL;
    }
  }

  bool entered() const { return entered_; }

private:
  fid_parser_plugin_instance *instance_;
  std::unique_lock<std::mutex> lock_;
  bool entered_;
};

bool cleanup_created_files_enabled() {
  return g_config != NULL &&
         (g_config->flags &
          FID_PARSER_PLUGIN_RUNTIME_CLEANUP_CREATED_FILES) != 0;
}

void cleanup_instance_files(fid_parser_plugin_instance *instance) noexcept {
  if (instance == NULL) {
    return;
  }
  instance->files.clear();
  for (const auto &path : instance->created_files) {
    if (!path.empty()) {
      (void)::unlink(path.c_str());
    }
  }
  instance->created_files.clear();
}

void discard_pending_instance(fid_parser_plugin_instance *instance) noexcept {
  if (instance == NULL) {
    return;
  }
  if (instance->enclave_id != 0) {
    (void)sgx_destroy_enclave(instance->enclave_id);
    instance->enclave_id = 0;
  }
  cleanup_instance_files(instance);
  if (g_config != NULL && g_config->destroy_context != NULL &&
      instance->instance_context != NULL) {
    try {
      g_config->destroy_context(instance->instance_context);
    } catch (...) {
    }
    instance->instance_context = NULL;
  }
}

void create_instance(const char *signed_enclave_path,
                     const fid_parser_host_callbacks *callbacks,
                     fid_parser_plugin_instance **instance_out,
                     fid_parser_call_status *status) {
  if (instance_out != NULL) {
    *instance_out = NULL;
  }
  if (!reset_status(status)) {
    return;
  }
  if (signed_enclave_path == NULL || signed_enclave_path[0] == '\0' ||
      instance_out == NULL || !callbacks_are_valid(callbacks)) {
    set_plugin_error(status, FID_PARSER_PLUGIN_INVALID_ARGUMENT);
    return;
  }

  std::unique_ptr<fid_parser_plugin_instance> instance;
  try {
    std::lock_guard<std::mutex> guard(g_runtime_mutex);
    if (g_config == NULL) {
      set_plugin_error(status, FID_PARSER_PLUGIN_INTERNAL_ERROR);
      return;
    }
    if (g_live_instance != NULL) {
      set_plugin_error(status, FID_PARSER_PLUGIN_BUSY);
      return;
    }

    instance.reset(new fid_parser_plugin_instance());
    instance->callbacks = *callbacks;
    instance->callbacks.struct_size = sizeof(instance->callbacks);

    if (g_config->create_context != NULL) {
      instance->instance_context =
          g_config->create_context(&instance->callbacks, status);
      if (status->plugin_status != FID_PARSER_PLUGIN_OK) {
        discard_pending_instance(instance.get());
        return;
      }
    }

    sgx_status_t sgx_result =
        sgx_create_enclave(signed_enclave_path, FID_PARSER_ENCLAVE_DEBUG_FLAG, NULL, NULL,
                           &instance->enclave_id, NULL);
    status->sgx_status = static_cast<uint32_t>(sgx_result);
    if (sgx_result != SGX_SUCCESS) {
      discard_pending_instance(instance.get());
      return;
    }

    g_live_instance = instance.get();
    *instance_out = instance.release();
  } catch (...) {
    discard_pending_instance(instance.get());
    set_plugin_error(status, FID_PARSER_PLUGIN_EXCEPTION);
    *instance_out = NULL;
  }
}

void destroy_instance(fid_parser_plugin_instance *instance,
                      fid_parser_call_status *status) {
  if (!reset_status(status)) {
    return;
  }
  if (instance == NULL) {
    set_plugin_error(status, FID_PARSER_PLUGIN_INVALID_ARGUMENT);
    return;
  }

  try {
    std::unique_lock<std::mutex> call_lock(instance->inflight,
                                           std::try_to_lock);
    if (!call_lock.owns_lock() || g_active_instance == instance) {
      set_plugin_error(status, FID_PARSER_PLUGIN_BUSY);
      return;
    }
    {
      std::lock_guard<std::mutex> guard(g_runtime_mutex);
      if (g_live_instance != instance || instance->destroying) {
        set_plugin_error(status, FID_PARSER_PLUGIN_NO_ACTIVE_INSTANCE);
        return;
      }
      instance->destroying = true;
      g_live_instance = NULL;
    }

    if (instance->enclave_id != 0) {
      status->sgx_status =
          static_cast<uint32_t>(sgx_destroy_enclave(instance->enclave_id));
      instance->enclave_id = 0;
    }
    cleanup_instance_files(instance);
    instance->storage.clear();
    if (g_config != NULL && g_config->destroy_context != NULL &&
        instance->instance_context != NULL) {
      try {
        g_config->destroy_context(instance->instance_context);
      } catch (...) {
        set_plugin_error(status, FID_PARSER_PLUGIN_EXCEPTION);
      }
      instance->instance_context = NULL;
    }
    std::memset(&instance->callbacks, 0, sizeof(instance->callbacks));
    call_lock.unlock();
    delete instance;
  } catch (...) {
    set_plugin_error(status, FID_PARSER_PLUGIN_EXCEPTION);
  }
}

void call_noarg(fid_parser_plugin_instance *instance,
                fid_parser_ecall_noarg_fn function,
                fid_parser_call_status *status) {
  if (!reset_status(status)) {
    return;
  }
  try {
    EcallScope scope(instance, status);
    if (!scope.entered()) {
      return;
    }
    uint32_t enclave_result = 0;
    const sgx_status_t sgx_result =
        function(instance->enclave_id, &enclave_result);
    status->sgx_status = static_cast<uint32_t>(sgx_result);
    if (sgx_result == SGX_SUCCESS) {
      status->enclave_status = enclave_result;
    }
  } catch (...) {
    set_plugin_error(status, FID_PARSER_PLUGIN_EXCEPTION);
  }
}

void begin_parse(fid_parser_plugin_instance *instance,
                 fid_parser_call_status *status) {
  call_noarg(instance, g_config->begin_parse_data_item, status);
}

void end_parse(fid_parser_plugin_instance *instance,
               fid_parser_call_status *status) {
  call_noarg(instance, g_config->end_parse_data_item, status);
}

void call_input(fid_parser_plugin_instance *instance,
                fid_parser_ecall_input_fn function, const uint8_t *data,
                uint32_t data_size, fid_parser_call_status *status) {
  if (!reset_status(status)) {
    return;
  }
  if (data == NULL && data_size != 0) {
    set_plugin_error(status, FID_PARSER_PLUGIN_INVALID_ARGUMENT);
    return;
  }
  try {
    EcallScope scope(instance, status);
    if (!scope.entered()) {
      return;
    }
    uint32_t enclave_result = 0;
    const sgx_status_t sgx_result =
        function(instance->enclave_id, &enclave_result, data, data_size);
    status->sgx_status = static_cast<uint32_t>(sgx_result);
    if (sgx_result == SGX_SUCCESS) {
      status->enclave_status = enclave_result;
    }
  } catch (...) {
    set_plugin_error(status, FID_PARSER_PLUGIN_EXCEPTION);
  }
}

void parse_data(fid_parser_plugin_instance *instance, const uint8_t *data,
                uint32_t data_size, fid_parser_call_status *status) {
  call_input(instance, g_config->parse_data_item, data, data_size, status);
}

void init_data_source(fid_parser_plugin_instance *instance,
                      const uint8_t *data, uint32_t data_size,
                      fid_parser_call_status *status) {
  call_input(instance, g_config->init_data_source, data, data_size, status);
}

void init_model(fid_parser_plugin_instance *instance, const uint8_t *data,
                uint32_t data_size, fid_parser_call_status *status) {
  call_input(instance, g_config->init_model, data, data_size, status);
}

void get_parser_type(fid_parser_plugin_instance *instance,
                     uint32_t *parser_type_out,
                     fid_parser_call_status *status) {
  if (parser_type_out != NULL) {
    *parser_type_out = 0;
  }
  if (!reset_status(status)) {
    return;
  }
  if (parser_type_out == NULL) {
    set_plugin_error(status, FID_PARSER_PLUGIN_INVALID_ARGUMENT);
    return;
  }
  try {
    EcallScope scope(instance, status);
    if (!scope.entered()) {
      return;
    }
    uint32_t parser_type = 0;
    const sgx_status_t sgx_result =
        g_config->get_parser_type(instance->enclave_id, &parser_type);
    status->sgx_status = static_cast<uint32_t>(sgx_result);
    if (sgx_result == SGX_SUCCESS) {
      *parser_type_out = parser_type;
      status->enclave_status = 0;
    }
  } catch (...) {
    set_plugin_error(status, FID_PARSER_PLUGIN_EXCEPTION);
  }
}

void get_output(fid_parser_plugin_instance *instance,
                fid_parser_ecall_noarg_fn size_function,
                fid_parser_ecall_output_fn output_function, uint8_t *buffer,
                uint32_t capacity, uint32_t *required_size,
                fid_parser_call_status *status) {
  if (required_size != NULL) {
    *required_size = 0;
  }
  if (!reset_status(status)) {
    return;
  }
  if (required_size == NULL || (buffer == NULL && capacity != 0)) {
    set_plugin_error(status, FID_PARSER_PLUGIN_INVALID_ARGUMENT);
    return;
  }

  try {
    EcallScope scope(instance, status);
    if (!scope.entered()) {
      return;
    }
    uint32_t size = 0;
    sgx_status_t sgx_result =
        size_function(instance->enclave_id, &size);
    status->sgx_status = static_cast<uint32_t>(sgx_result);
    if (sgx_result != SGX_SUCCESS) {
      return;
    }
    *required_size = size;
    status->enclave_status = 0;

    if (buffer == NULL && capacity == 0) {
      if (size != 0) {
        set_plugin_error(status, FID_PARSER_PLUGIN_BUFFER_TOO_SMALL);
      }
      return;
    }
    if (capacity < size) {
      set_plugin_error(status, FID_PARSER_PLUGIN_BUFFER_TOO_SMALL);
      return;
    }
    if (size == 0) {
      return;
    }

    uint32_t enclave_result = 0;
    sgx_result = output_function(instance->enclave_id, &enclave_result,
                                 buffer, size);
    status->sgx_status = static_cast<uint32_t>(sgx_result);
    if (sgx_result != SGX_SUCCESS) {
      *required_size = 0;
      return;
    }
    status->enclave_status = enclave_result;
    if (enclave_result != 0) {
      *required_size = 0;
    }
  } catch (...) {
    if (required_size != NULL) {
      *required_size = 0;
    }
    set_plugin_error(status, FID_PARSER_PLUGIN_EXCEPTION);
  }
}

void get_enclave_hash(fid_parser_plugin_instance *instance, uint8_t *buffer,
                      uint32_t capacity, uint32_t *required_size,
                      fid_parser_call_status *status) {
  get_output(instance, g_config->get_enclave_hash_size,
             g_config->get_enclave_hash, buffer, capacity, required_size,
             status);
}

void get_analyze_result(fid_parser_plugin_instance *instance, uint8_t *buffer,
                        uint32_t capacity, uint32_t *required_size,
                        fid_parser_call_status *status) {
  get_output(instance, g_config->get_analyze_result_size,
             g_config->get_analyze_result, buffer, capacity, required_size,
             status);
}

fid_parser_plugin_api make_api(
    const fid_parser_plugin_runtime_config *config) {
  fid_parser_plugin_api api = {};
  api.struct_size = sizeof(api);
  api.abi_major = FID_PARSER_PLUGIN_ABI_MAJOR;
  api.abi_minor = FID_PARSER_PLUGIN_ABI_MINOR;
  api.module_id = config->module_id;
  api.edl_contract_fingerprint = config->edl_contract_fingerprint;
  api.flags =
      (config->flags & ~FID_PARSER_PLUGIN_RUNTIME_CLEANUP_CREATED_FILES) |
              FID_PARSER_PLUGIN_FLAG_SINGLE_ACTIVE_INSTANCE |
              FID_PARSER_PLUGIN_FLAG_SINGLE_INFLIGHT_ECALL |
              FID_PARSER_PLUGIN_FLAG_NO_SWITCHLESS_OCALL;
  api.create = create_instance;
  api.destroy = destroy_instance;
  api.begin_parse_data_item = begin_parse;
  api.parse_data_item = parse_data;
  api.end_parse_data_item = end_parse;
  api.init_data_source = init_data_source;
  api.init_model = init_model;
  api.get_parser_type = get_parser_type;
  api.get_enclave_hash = get_enclave_hash;
  api.get_analyze_result = get_analyze_result;
  return api;
}

fid_parser_plugin_instance *active_instance() { return g_active_instance; }

std::array<uint8_t, 32> storage_key(const uint8_t *key) {
  std::array<uint8_t, 32> result;
  std::copy(key, key + result.size(), result.begin());
  return result;
}

std::ios_base::seekdir seek_direction(uint8_t direction, bool *valid) {
  *valid = true;
  if (direction == ypc::ios_base::beg) {
    return std::ios_base::beg;
  }
  if (direction == ypc::ios_base::end) {
    return std::ios_base::end;
  }
  if (direction == ypc::ios_base::cur) {
    return std::ios_base::cur;
  }
  *valid = false;
  return std::ios_base::beg;
}

} // namespace

extern "C" int32_t fid_parser_plugin_runtime_query(
    const fid_parser_plugin_runtime_config *config,
    uint32_t host_abi_major, uint32_t host_abi_minor,
    uint32_t host_api_struct_size, const fid_parser_plugin_api **api_out) {
  if (api_out != NULL) {
    *api_out = NULL;
  }
  if (api_out == NULL || !config_is_valid(config)) {
    return FID_PARSER_PLUGIN_INVALID_ARGUMENT;
  }
  // Minor revisions are append-only within one ABI major. The descriptor
  // size, rather than exact minor equality, determines whether the v1 prefix
  // required by this plugin can be exchanged with the host.
  (void)host_abi_minor;
  if (host_abi_major != FID_PARSER_PLUGIN_ABI_MAJOR ||
      host_api_struct_size < FID_PARSER_PLUGIN_API_V1_0_SIZE) {
    return FID_PARSER_PLUGIN_ABI_MISMATCH;
  }

  try {
    std::lock_guard<std::mutex> guard(g_runtime_mutex);
    if (g_config != NULL && g_config != config) {
      return FID_PARSER_PLUGIN_INTERNAL_ERROR;
    }
    if (g_config == NULL) {
      g_config = config;
      g_api = make_api(config);
    }
    *api_out = &g_api;
    return FID_PARSER_PLUGIN_OK;
  } catch (...) {
    *api_out = NULL;
    return FID_PARSER_PLUGIN_EXCEPTION;
  }
}

extern "C" void *fid_parser_plugin_runtime_active_context(void) {
  return active_instance() == NULL ? NULL : active_instance()->instance_context;
}

extern "C" const fid_parser_host_callbacks *
fid_parser_plugin_runtime_active_host_callbacks(void) {
  return active_instance() == NULL ? NULL : &active_instance()->callbacks;
}

extern "C" uint32_t next_data_batch(const uint8_t *data_hash,
                                     uint32_t hash_size, uint8_t **data,
                                     uint32_t *len) {
  if (data != NULL) {
    *data = NULL;
  }
  if (len != NULL) {
    *len = 0;
  }
  fid_parser_plugin_instance *instance = active_instance();
  if (instance == NULL || data == NULL || len == NULL || data_hash == NULL ||
      hash_size < 32 || instance->callbacks.next_data_batch == NULL) {
    return kHostCallbackError;
  }
  try {
    const uint32_t result = instance->callbacks.next_data_batch(
        instance->callbacks.host_context, data_hash, hash_size, data, len);
    if ((*len != 0 && *data == NULL)) {
      *data = NULL;
      *len = 0;
      return kHostCallbackError;
    }
    return result;
  } catch (...) {
    *data = NULL;
    *len = 0;
    return kHostCallbackError;
  }
}

extern "C" uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1,
                                               uint32_t *session_id) {
  if (dh_msg1 != NULL) {
    std::memset(dh_msg1, 0, sizeof(*dh_msg1));
  }
  if (session_id != NULL) {
    *session_id = 0;
  }
  fid_parser_plugin_instance *instance = active_instance();
  if (instance == NULL || dh_msg1 == NULL || session_id == NULL ||
      instance->callbacks.km_session_request == NULL) {
    return kHostCallbackError;
  }
  try {
    return instance->callbacks.km_session_request(
        instance->callbacks.host_context, dh_msg1, session_id);
  } catch (...) {
    std::memset(dh_msg1, 0, sizeof(*dh_msg1));
    *session_id = 0;
    return kHostCallbackError;
  }
}

extern "C" uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                               sgx_dh_msg3_t *dh_msg3,
                                               uint32_t session_id) {
  if (dh_msg3 != NULL) {
    std::memset(dh_msg3, 0, sizeof(*dh_msg3));
  }
  fid_parser_plugin_instance *instance = active_instance();
  if (instance == NULL || dh_msg2 == NULL || dh_msg3 == NULL ||
      instance->callbacks.km_exchange_report == NULL) {
    return kHostCallbackError;
  }
  try {
    return instance->callbacks.km_exchange_report(
        instance->callbacks.host_context, dh_msg2, dh_msg3, session_id);
  } catch (...) {
    std::memset(dh_msg3, 0, sizeof(*dh_msg3));
    return kHostCallbackError;
  }
}

extern "C" uint32_t km_send_request_ocall(
    uint32_t session_id, secure_message_t *req_message,
    std::size_t req_message_size, std::size_t max_payload_size,
    secure_message_t *resp_message, std::size_t resp_message_size) {
  if (resp_message != NULL && resp_message_size != 0) {
    std::memset(resp_message, 0, resp_message_size);
  }
  fid_parser_plugin_instance *instance = active_instance();
  if (instance == NULL || req_message == NULL || resp_message == NULL ||
      instance->callbacks.km_send_request == NULL) {
    return kHostCallbackError;
  }
  try {
    return instance->callbacks.km_send_request(
        instance->callbacks.host_context, session_id, req_message,
        static_cast<uint64_t>(req_message_size),
        static_cast<uint64_t>(max_payload_size), resp_message,
        static_cast<uint64_t>(resp_message_size));
  } catch (...) {
    if (resp_message_size != 0) {
      std::memset(resp_message, 0, resp_message_size);
    }
    return kHostCallbackError;
  }
}

extern "C" uint32_t km_end_session_ocall(uint32_t session_id) {
  fid_parser_plugin_instance *instance = active_instance();
  if (instance == NULL || instance->callbacks.km_end_session == NULL) {
    return kHostCallbackError;
  }
  try {
    return instance->callbacks.km_end_session(
        instance->callbacks.host_context, session_id);
  } catch (...) {
    return kHostCallbackError;
  }
}

extern "C" void ocall_print_string(const char *message) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    const std::size_t size = message == NULL ? 0 : std::strlen(message);
    if (instance != NULL && instance->callbacks.log != NULL) {
      instance->callbacks.log(instance->callbacks.host_context, 0, message,
                              static_cast<uint32_t>(size));
    } else if (message != NULL) {
      std::fwrite(message, 1, size, stderr);
      std::fflush(stderr);
    }
  } catch (...) {
  }
}

extern "C" void ocall_log_string(uint32_t rank, const char *message) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    const std::size_t size = message == NULL ? 0 : std::strlen(message);
    if (instance != NULL && instance->callbacks.log != NULL) {
      instance->callbacks.log(instance->callbacks.host_context, rank, message,
                              static_cast<uint32_t>(size));
    } else if (message != NULL) {
      std::fwrite(message, 1, size, stderr);
      std::fputc('\n', stderr);
      std::fflush(stderr);
    }
  } catch (...) {
  }
}

extern "C" uint32_t fopen_ocall(const char *filename, std::size_t len,
                                 uint32_t mode) {
  fid_parser_plugin_instance *instance = active_instance();
  if (instance == NULL || filename == NULL) {
    return 0;
  }
  try {
    std::ios_base::openmode open_mode = static_cast<std::ios_base::openmode>(0);
    if ((mode & ypc::ios_base::app) != 0) open_mode |= std::ios_base::app;
    if ((mode & ypc::ios_base::ate) != 0) open_mode |= std::ios_base::ate;
    if ((mode & ypc::ios_base::binary) != 0) open_mode |= std::ios_base::binary;
    if ((mode & ypc::ios_base::in) != 0) open_mode |= std::ios_base::in;
    if ((mode & ypc::ios_base::out) != 0) open_mode |= std::ios_base::out;
    if ((mode & ypc::ios_base::trunc) != 0) open_mode |= std::ios_base::trunc;

    const std::string path(filename, len);
    std::unique_ptr<std::fstream> stream(new std::fstream());
    stream->open(path.c_str(), open_mode);
    if (!stream->is_open()) {
      return 0;
    }
    if (cleanup_created_files_enabled() &&
        (mode & (ypc::ios_base::out | ypc::ios_base::trunc)) != 0) {
      try {
        instance->created_files.insert(path);
      } catch (...) {
        stream->close();
        (void)::unlink(path.c_str());
        return 0;
      }
    }
    uint32_t id = instance->next_stream_id++;
    if (id == 0) {
      id = instance->next_stream_id++;
    }
    instance->files.insert(std::make_pair(id, std::move(stream)));
    return id;
  } catch (...) {
    return 0;
  }
}

extern "C" void fclose_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL) return;
    std::unordered_map<uint32_t, std::unique_ptr<std::fstream> >::iterator it =
        instance->files.find(stream);
    if (it == instance->files.end()) return;
    it->second->close();
    instance->files.erase(it);
  } catch (...) {
  }
}

extern "C" void fflush_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || instance->files.count(stream) == 0) return;
    instance->files[stream]->flush();
  } catch (...) {
  }
}

extern "C" void fread_ocall(void *ptr, std::size_t size, uint32_t stream) {
  if (ptr != NULL && size != 0) {
    std::memset(ptr, 0, size);
  }
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || (ptr == NULL && size != 0) ||
        instance->files.count(stream) == 0) return;
    instance->files[stream]->read(static_cast<char *>(ptr), size);
  } catch (...) {
  }
}

extern "C" void fwrite_ocall(const void *ptr, std::size_t size,
                              uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || (ptr == NULL && size != 0) ||
        instance->files.count(stream) == 0) return;
    instance->files[stream]->write(static_cast<const char *>(ptr), size);
  } catch (...) {
  }
}

extern "C" void fseekg_ocall(uint32_t stream, int64_t offset, uint8_t dir) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || instance->files.count(stream) == 0) return;
    bool valid = false;
    const std::ios_base::seekdir direction = seek_direction(dir, &valid);
    if (valid) instance->files[stream]->seekg(offset, direction);
  } catch (...) {
  }
}

extern "C" int64_t ftellg_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || instance->files.count(stream) == 0) return -1;
    return static_cast<int64_t>(instance->files[stream]->tellg());
  } catch (...) {
    return -1;
  }
}

extern "C" void fseekp_ocall(uint32_t stream, int64_t offset, uint8_t dir) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || instance->files.count(stream) == 0) return;
    bool valid = false;
    const std::ios_base::seekdir direction = seek_direction(dir, &valid);
    if (valid) instance->files[stream]->seekp(offset, direction);
  } catch (...) {
  }
}

extern "C" int64_t ftellp_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || instance->files.count(stream) == 0) return -1;
    return static_cast<int64_t>(instance->files[stream]->tellp());
  } catch (...) {
    return -1;
  }
}

extern "C" uint8_t feof_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    return instance == NULL || instance->files.count(stream) == 0
               ? 1
               : static_cast<uint8_t>(instance->files[stream]->eof());
  } catch (...) {
    return 1;
  }
}

extern "C" uint8_t fgood_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    return instance == NULL || instance->files.count(stream) == 0
               ? 0
               : static_cast<uint8_t>(instance->files[stream]->good());
  } catch (...) {
    return 0;
  }
}

extern "C" uint8_t ffail_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    return instance == NULL || instance->files.count(stream) == 0
               ? 1
               : static_cast<uint8_t>(instance->files[stream]->fail());
  } catch (...) {
    return 1;
  }
}

extern "C" uint8_t fbad_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    return instance == NULL || instance->files.count(stream) == 0
               ? 1
               : static_cast<uint8_t>(instance->files[stream]->bad());
  } catch (...) {
    return 1;
  }
}

extern "C" void clear_ocall(uint32_t stream) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance != NULL && instance->files.count(stream) != 0)
      instance->files[stream]->clear();
  } catch (...) {
  }
}

extern "C" uint8_t has_key_ocall(const uint8_t *key) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || key == NULL) return 0;
    return static_cast<uint8_t>(instance->storage.count(storage_key(key)) != 0);
  } catch (...) {
    return 0;
  }
}

extern "C" uint32_t remove_key_ocall(const uint8_t *key) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || key == NULL) return kHostCallbackError;
    instance->storage.erase(storage_key(key));
    return 0;
  } catch (...) {
    return kHostCallbackError;
  }
}

extern "C" uint32_t write_to_storage_ocall(const uint8_t *key,
                                             const uint8_t *value,
                                             std::size_t len) {
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || key == NULL || (value == NULL && len != 0))
      return kHostCallbackError;
    if (len == 0) {
      instance->storage[storage_key(key)].clear();
    } else {
      instance->storage[storage_key(key)] =
          std::vector<uint8_t>(value, value + len);
    }
    return 0;
  } catch (...) {
    return kHostCallbackError;
  }
}

extern "C" uint32_t read_from_storage_ocall(const uint8_t *key,
                                              uint8_t *value,
                                              std::size_t data_size) {
  if (value != NULL && data_size != 0) {
    std::memset(value, 0, data_size);
  }
  try {
    fid_parser_plugin_instance *instance = active_instance();
    if (instance == NULL || key == NULL || (value == NULL && data_size != 0))
      return kHostCallbackError;
    const std::array<uint8_t, 32> storage_key_value = storage_key(key);
    std::unordered_map<std::array<uint8_t, 32>, std::vector<uint8_t>,
                       KeyHash>::const_iterator it =
        instance->storage.find(storage_key_value);
    if (it == instance->storage.end()) return 1;
    if (it->second.size() != data_size) return 2;
    if (data_size != 0) std::memcpy(value, it->second.data(), data_size);
    return 0;
  } catch (...) {
    if (value != NULL && data_size != 0) std::memset(value, 0, data_size);
    return kHostCallbackError;
  }
}
