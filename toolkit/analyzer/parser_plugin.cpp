#include "parser_plugin.h"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sgx_error.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

namespace fid {
namespace {

class scoped_descriptor final {
public:
  explicit scoped_descriptor(int descriptor) : descriptor_(descriptor) {}
  scoped_descriptor(const scoped_descriptor &) = delete;
  scoped_descriptor &operator=(const scoped_descriptor &) = delete;
  scoped_descriptor(scoped_descriptor &&) = delete;
  scoped_descriptor &operator=(scoped_descriptor &&) = delete;
  ~scoped_descriptor() {
    if (descriptor_ >= 0) {
      (void)::close(descriptor_);
    }
  }
  int get() const noexcept { return descriptor_; }
  int release() noexcept {
    const int descriptor = descriptor_;
    descriptor_ = -1;
    return descriptor;
  }

private:
  int descriptor_;
};

std::string call_error(const char *name, const fid_parser_call_status &status) {
  std::ostringstream message;
  message << name << " failed: plugin_status=" << status.plugin_status
          << ", sgx_status=0x" << std::hex << status.sgx_status;
  return message.str();
}

void require_api(const fid_parser_plugin_api *api) {
  const size_t minimum_size = FID_PARSER_PLUGIN_API_V1_0_SIZE;
  // Minor revisions are append-only. Accept any same-major descriptor that
  // contains the complete v1 prefix consumed below.
  if (api == nullptr || api->struct_size < minimum_size ||
      api->abi_major != FID_PARSER_PLUGIN_ABI_MAJOR ||
      api->module_id == nullptr || api->edl_contract_fingerprint == nullptr ||
      api->create == nullptr || api->destroy == nullptr ||
      api->begin_parse_data_item == nullptr ||
      api->parse_data_item == nullptr || api->end_parse_data_item == nullptr ||
      api->init_data_source == nullptr || api->init_model == nullptr ||
      api->get_parser_type == nullptr || api->get_enclave_hash == nullptr ||
      api->get_analyze_result == nullptr) {
    throw std::runtime_error(
        "parser plugin returned an invalid ABI descriptor");
  }
  const uint32_t required_flags =
      FID_PARSER_PLUGIN_FLAG_SINGLE_ACTIVE_INSTANCE |
      FID_PARSER_PLUGIN_FLAG_SINGLE_INFLIGHT_ECALL |
      FID_PARSER_PLUGIN_FLAG_NO_SWITCHLESS_OCALL;
  if ((api->flags & required_flags) != required_flags) {
    throw std::runtime_error(
        "parser plugin does not declare the required v1 execution limits");
  }
}

} // namespace

parser_plugin::parser_plugin(const parser_registry_selection &selection,
                             const fid_parser_host_callbacks &callbacks)
    : m_api(nullptr), m_instance(nullptr), m_dso(nullptr),
      m_enclave_descriptor(-1) {
  if (selection.module.abi_major != FID_PARSER_PLUGIN_ABI_MAJOR) {
    throw std::runtime_error("registry parser plugin ABI is not supported");
  }

  const int module_descriptor = ::open(selection.module.library_path.c_str(),
                                       O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
  if (module_descriptor < 0) {
    throw std::runtime_error("cannot open parser plugin: " +
                             std::string(std::strerror(errno)));
  }
  scoped_descriptor module_file(module_descriptor);
  parser_registry::require_trusted_fd(module_file.get(), "parser module");
  if (parser_registry::sha256_fd(module_file.get()) !=
      selection.module.library_sha256) {
    throw std::runtime_error("parser module SHA-256 mismatch: " +
                             selection.module.library_path);
  }

  const int enclave_descriptor =
      ::open(selection.enclave_path.c_str(),
             O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
  if (enclave_descriptor < 0) {
    throw std::runtime_error("cannot open signed enclave: " +
                             std::string(std::strerror(errno)));
  }
  scoped_descriptor enclave_file(enclave_descriptor);
  parser_registry::require_trusted_fd(enclave_file.get(), "signed enclave",
                                      true);
  if (parser_registry::sha256_fd(enclave_file.get()) !=
      selection.enclave.enclave_sha256) {
    throw std::runtime_error("signed enclave SHA-256 changed before load: " +
                             selection.enclave_path);
  }
  const std::string enclave_descriptor_path =
      "/proc/self/fd/" + std::to_string(enclave_file.get());

  const std::string descriptor_path =
      "/proc/self/fd/" + std::to_string(module_file.get());
  ::dlerror();
  m_dso = ::dlopen(descriptor_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (m_dso == nullptr) {
    const char *error = ::dlerror();
    throw std::runtime_error(
        std::string("cannot load parser plugin: ") +
        (error == nullptr ? "unknown dlopen error" : error));
  }

  ::dlerror();
  void *symbol = ::dlsym(m_dso, FID_PARSER_PLUGIN_QUERY_SYMBOL);
  const char *symbol_error = ::dlerror();
  if (symbol_error != nullptr || symbol == nullptr) {
    throw std::runtime_error(
        std::string("parser plugin has no query factory: ") +
        (symbol_error == nullptr ? "symbol not found" : symbol_error));
  }
  auto query = reinterpret_cast<fid_parser_plugin_query_fn>(symbol);
  const int32_t query_status =
      query(FID_PARSER_PLUGIN_ABI_MAJOR, FID_PARSER_PLUGIN_ABI_MINOR,
            sizeof(fid_parser_plugin_api), &m_api);
  if (query_status != FID_PARSER_PLUGIN_OK) {
    throw std::runtime_error("parser plugin rejected host ABI: " +
                             std::to_string(query_status));
  }
  require_api(m_api);

  if (selection.module.module_id != m_api->module_id ||
      selection.module.edl_fingerprint !=
          normalize_hex(m_api->edl_contract_fingerprint)) {
    throw std::runtime_error(
        "parser plugin descriptor does not match its registry fragment");
  }

  fid_parser_call_status status = {sizeof(fid_parser_call_status),
                                   FID_PARSER_PLUGIN_INTERNAL_ERROR,
                                   SGX_ERROR_UNEXPECTED, 0};
  m_api->create(enclave_descriptor_path.c_str(), &callbacks, &m_instance,
                &status);
  try {
    checked_business_status("create", status);
  } catch (...) {
    if (m_api != nullptr && m_instance != nullptr) {
      fid_parser_call_status destroy_status = {
          sizeof(fid_parser_call_status), FID_PARSER_PLUGIN_INTERNAL_ERROR,
          SGX_ERROR_UNEXPECTED, 0};
      m_api->destroy(m_instance, &destroy_status);
      m_instance = nullptr;
    }
    throw;
  }
  if (m_instance == nullptr) {
    throw std::runtime_error(
        "parser plugin create succeeded without returning an instance");
  }
  // Keep the exact file that was hashed open for the entire enclave instance
  // lifetime. URTS loaded this same inode through /proc/self/fd above, so a
  // path replacement cannot substitute another enclave after verification.
  m_enclave_descriptor = enclave_file.release();
}

parser_plugin::~parser_plugin() {
  if (m_api != nullptr && m_instance != nullptr) {
    fid_parser_call_status status = {sizeof(fid_parser_call_status),
                                     FID_PARSER_PLUGIN_INTERNAL_ERROR,
                                     SGX_ERROR_UNEXPECTED, 0};
    m_api->destroy(m_instance, &status);
    if (status.plugin_status != FID_PARSER_PLUGIN_OK ||
        status.sgx_status != SGX_SUCCESS) {
      std::cerr << call_error("parser plugin destroy", status) << std::endl;
    }
    m_instance = nullptr;
  }
  if (m_enclave_descriptor >= 0) {
    (void)::close(m_enclave_descriptor);
    m_enclave_descriptor = -1;
  }
  // Intentionally do not dlclose. Descriptor pointers and Edger8r OCALL
  // trampolines must remain valid until process exit.
}

uint32_t
parser_plugin::checked_business_status(const char *name,
                                       const fid_parser_call_status &status) {
  if (status.struct_size < sizeof(fid_parser_call_status) ||
      status.plugin_status != FID_PARSER_PLUGIN_OK ||
      status.sgx_status != SGX_SUCCESS) {
    throw std::runtime_error(call_error(name, status));
  }
  return status.enclave_status;
}

uint32_t parser_plugin::invoke(const char *name, simple_call fn) {
  fid_parser_call_status status = {sizeof(fid_parser_call_status),
                                   FID_PARSER_PLUGIN_INTERNAL_ERROR,
                                   SGX_ERROR_UNEXPECTED, 0};
  fn(m_instance, &status);
  return checked_business_status(name, status);
}

uint32_t parser_plugin::invoke_data(const char *name,
                                    void (*fn)(fid_parser_plugin_instance *,
                                               const uint8_t *, uint32_t,
                                               fid_parser_call_status *),
                                    const uint8_t *data, uint32_t size) {
  fid_parser_call_status status = {sizeof(fid_parser_call_status),
                                   FID_PARSER_PLUGIN_INTERNAL_ERROR,
                                   SGX_ERROR_UNEXPECTED, 0};
  fn(m_instance, data, size, &status);
  return checked_business_status(name, status);
}

uint32_t parser_plugin::invoke_buffer(const char *name,
                                      void (*fn)(fid_parser_plugin_instance *,
                                                 uint8_t *, uint32_t,
                                                 uint32_t *,
                                                 fid_parser_call_status *),
                                      ypc::bytes &value) {
  value = ypc::bytes();
  uint32_t required = 0;
  fid_parser_call_status status = {sizeof(fid_parser_call_status),
                                   FID_PARSER_PLUGIN_INTERNAL_ERROR,
                                   SGX_ERROR_UNEXPECTED, 0};
  fn(m_instance, nullptr, 0, &required, &status);
  if (status.plugin_status != FID_PARSER_PLUGIN_BUFFER_TOO_SMALL) {
    const uint32_t business = checked_business_status(name, status);
    if (business != 0 || required == 0) {
      return business;
    }
  }
  if (required == 0) {
    throw std::runtime_error(std::string(name) +
                             " returned BUFFER_TOO_SMALL with zero length");
  }

  std::vector<uint8_t> buffer(required);
  status = {sizeof(fid_parser_call_status), FID_PARSER_PLUGIN_INTERNAL_ERROR,
            SGX_ERROR_UNEXPECTED, 0};
  uint32_t final_required = 0;
  fn(m_instance, buffer.data(), static_cast<uint32_t>(buffer.size()),
     &final_required, &status);
  const uint32_t business = checked_business_status(name, status);
  if (business != 0) {
    return business;
  }
  if (final_required > buffer.size()) {
    throw std::runtime_error(std::string(name) +
                             " grew beyond the caller-owned buffer");
  }
  value = ypc::bytes(buffer.data(), final_required);
  return 0;
}

uint32_t parser_plugin::begin_parse_data_item() {
  return invoke("begin_parse_data_item", m_api->begin_parse_data_item);
}

uint32_t parser_plugin::parse_data_item(const uint8_t *data, uint32_t size) {
  return invoke_data("parse_data_item", m_api->parse_data_item, data, size);
}

uint32_t parser_plugin::end_parse_data_item() {
  return invoke("end_parse_data_item", m_api->end_parse_data_item);
}

uint32_t parser_plugin::init_data_source(const uint8_t *data, uint32_t size) {
  return invoke_data("init_data_source", m_api->init_data_source, data, size);
}

uint32_t parser_plugin::init_model(const uint8_t *data, uint32_t size) {
  return invoke_data("init_model", m_api->init_model, data, size);
}

uint32_t parser_plugin::get_parser_type(uint32_t &type) {
  type = 0;
  fid_parser_call_status status = {sizeof(fid_parser_call_status),
                                   FID_PARSER_PLUGIN_INTERNAL_ERROR,
                                   SGX_ERROR_UNEXPECTED, 0};
  m_api->get_parser_type(m_instance, &type, &status);
  return checked_business_status("get_parser_type", status);
}

uint32_t parser_plugin::get_enclave_hash(ypc::bytes &hash) {
  return invoke_buffer("get_enclave_hash", m_api->get_enclave_hash, hash);
}

uint32_t parser_plugin::get_analyze_result(ypc::bytes &result) {
  return invoke_buffer("get_analyze_result", m_api->get_analyze_result, result);
}

} // namespace fid
