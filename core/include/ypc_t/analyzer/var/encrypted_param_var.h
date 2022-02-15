#pragma once
#include "stbox/ebyte.h"

namespace ypc {
namespace internal {

class encrypted_param_var {
protected:
  stbox::bytes m_encrypted_param;
};
} // namespace internal
} // namespace ypc
