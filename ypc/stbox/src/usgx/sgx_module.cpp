#include "ypc/stbox/usgx/sgx_module.h"
#include "sgx_urts.h"
#include "ypc/stbox/usgx/error_message.h"
#include <glog/logging.h>
#include <iostream>

namespace stbox {
namespace internal {

void buffer_length_traits::call(sgx_module_base &m) {
  //*m_len = m.ecall<uint32_t>(m_func);
  *m_len = m_func->call(m);
  *m_mem = (uint8_t *)malloc(*m_len);
}

} // namespace internal
sgx_module::sgx_module(const char *mod_path)
    : internal::sgx_module_base(mod_path) {
  sgx_status_t ret = SGX_ERROR_UNEXPECTED;

  LOG(INFO) << "SGX_DEBUG_FLAG: " << SGX_DEBUG_FLAG;
  /* Call sgx_create_enclave to initialize an enclave instance */
  /* Debug Support: set 2nd parameter to 1 */
  ret = sgx_create_enclave(mod_path, SGX_DEBUG_FLAG, NULL, NULL, &m_sgx_eid,
                           NULL);
  if (ret != SGX_SUCCESS) {
    std::cerr << "sgx_create_enclave fail " << status_string(ret)
              << ", for file: " << mod_path << std::endl;
    throw std::runtime_error(std::to_string(ret));
  }
}

sgx_module::~sgx_module() { sgx_destroy_enclave(m_sgx_eid); }

} // namespace stbox
