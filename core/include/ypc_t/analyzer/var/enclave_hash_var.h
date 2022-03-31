#pragma once
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/analyzer_context.h"

namespace ypc {
namespace internal {
class enclave_hash_var : virtual public analyzer_context {
public:
  inline void set_enclave_hash(const uint8_t *hash, uint32_t hash_size) {
    m_enclave_hash = stbox::bytes(hash, hash_size);
  }

  virtual const stbox::bytes &get_enclave_hash() const;

protected:
  stbox::bytes m_enclave_hash;
};
} // namespace internal
} // namespace ypc
