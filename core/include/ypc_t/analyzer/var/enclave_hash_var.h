#pragma once
#include "stbox/ebyte.h"
namespace ypc {

namespace internal {
class enclave_hash_var {
public:
  inline void set_enclave_hash(const uint8_t *hash, uint32_t hash_size) {
    m_enclave_hash = stbox::bytes(hash, hash_size);
  }

protected:
  stbox::bytes m_enclave_hash;
};
} // namespace internal
} // namespace ypc
