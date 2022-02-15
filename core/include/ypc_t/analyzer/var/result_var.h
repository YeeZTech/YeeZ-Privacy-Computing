#pragma once
#include "stbox/ebyte.h"
namespace ypc {
namespace internal {
class result_var {
protected:
  stbox::bytes m_result;
  uint64_t m_cost_gas;
};
} // namespace internal
} // namespace ypc
