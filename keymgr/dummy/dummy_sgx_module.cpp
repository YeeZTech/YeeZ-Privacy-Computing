#include "dummy_sgx_module.h"
#include "EnclaveInitiator_u.h"
#include <sgx_urts.h>
#include <stdexcept>

dummy_sgx_module::dummy_sgx_module(const char *mod_path)
    : ::stbox::sgx_module(mod_path) {}
dummy_sgx_module::~dummy_sgx_module() {}

uint32_t dummy_sgx_module::create_session() {
  return ecall<uint32_t>(::create_session);
}
uint32_t dummy_sgx_module::request_forward(uint32_t msg_id) {
  return ecall<uint32_t>(::request_forward, msg_id);
}
uint32_t dummy_sgx_module::close_session() {
  return ecall<uint32_t>(::close_session);
}
