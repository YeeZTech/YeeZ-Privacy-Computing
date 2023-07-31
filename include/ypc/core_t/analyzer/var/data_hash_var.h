#pragma once
#include "ypc/stbox/ebyte.h"

namespace ypc {
namespace internal {
class data_hash_var {
protected:
  stbox::bytes m_data_hash;
};

class data_merkle_hash_var {
protected:
  std::vector<stbox::bytes> m_data_hash;
};

} // namespace internal
} // namespace ypc
