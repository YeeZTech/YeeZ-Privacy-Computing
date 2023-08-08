#pragma once
#include "ypc/stbox/ebyte.h"

namespace ypc {
namespace internal {
class intermediate_var {
protected:
  // m_data_kgt_pkey is serialized bytes
  stbox::bytes m_data_kgt_pkey;
  stbox::bytes m_algo_pkey;
  stbox::bytes m_user_pkey;
};
} // namespace internal
} // namespace ypc
