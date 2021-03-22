#include "stbox/usgx/error_message.h"
#include <sstream>

namespace stbox {
typedef struct _sgx_errlist_t {
  sgx_status_t err;
  const char *msg;
  const char *sug; /* Suggestion */
} sgx_errlist_t;
/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {SGX_ERROR_UNEXPECTED, "Unexpected error occurred.", NULL},
    {SGX_ERROR_INVALID_PARAMETER, "Invalid parameter.", NULL},
    {SGX_ERROR_OUT_OF_MEMORY, "Out of memory.", NULL},
    {SGX_ERROR_ENCLAVE_LOST, "Power transition occurred.",
     "Please refer to the sample \"PowerTransition\" for details."},
    {SGX_ERROR_INVALID_ENCLAVE, "Invalid enclave image.", NULL},
    {SGX_ERROR_INVALID_ENCLAVE_ID, "Invalid enclave identification.", NULL},
    {SGX_ERROR_INVALID_SIGNATURE, "Invalid enclave signature.", NULL},
    {SGX_ERROR_OUT_OF_EPC, "Out of EPC memory.", NULL},
    {SGX_ERROR_NO_DEVICE, "Invalid SGX device.",
     "Please make sure SGX module is enabled in the BIOS, and install SGX "
     "driver afterwards."},
    {SGX_ERROR_MEMORY_MAP_CONFLICT, "Memory map conflicted.", NULL},
    {SGX_ERROR_INVALID_METADATA, "Invalid enclave metadata.", NULL},
    {SGX_ERROR_DEVICE_BUSY, "SGX device was busy.", NULL},
    {SGX_ERROR_INVALID_VERSION, "Enclave version was invalid.", NULL},
    {SGX_ERROR_INVALID_ATTRIBUTE, "Enclave was not authorized.", NULL},
    {SGX_ERROR_ENCLAVE_FILE_ACCESS, "Can't open enclave file.", NULL},
};
} // namespace stbox

namespace std {
std::string to_string(sgx_status_t ret) {
  size_t idx = 0;
  size_t ttl = sizeof(stbox::sgx_errlist) / sizeof(stbox::sgx_errlist[0]);

  std::stringstream ss;
  for (idx = 0; idx < ttl; idx++) {
    if (ret == stbox::sgx_errlist[idx].err) {
      if (NULL != stbox::sgx_errlist[idx].sug) {
        ss << "Info: " << stbox::sgx_errlist[idx].sug << "\n";
      }
      ss << "Error: " << stbox::sgx_errlist[idx].msg << "\n";
      break;
    }
  }

  if (idx == ttl) {
    ss << "Error code is 0x" << std::hex << ret
       << ". Please refer to the \"Intel SGX SDK Developer Reference\" for "
          "more details.";
  }
  return ss.str();
}

} // namespace std
