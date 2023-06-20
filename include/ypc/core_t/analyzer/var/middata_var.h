#pragma once
#include "ypc/stbox/ebyte.h"

namespace ypc {
namespace internal {
class middata_var {
protected:
  std::vector<stbox::bytes> m_data_pkey;
  stbox::bytes m_algo_pkey;
  stbox::bytes m_mid_pkey;
};
} // namespace internal
} // namespace ypc