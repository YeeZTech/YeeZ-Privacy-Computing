#include "ypc_t/analyzer/var/enclave_hash_var.h"

namespace ypc {
namespace internal {
const stbox::bytes &enclave_hash_var::get_enclave_hash() const {
  return m_enclave_hash;
}
} // namespace internal
} // namespace ypc
