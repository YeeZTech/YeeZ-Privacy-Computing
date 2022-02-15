#pragma once
#include "stbox/ebyte.h"

namespace ypc {
namespace internal {

template <bool input_encrypted> class request_key_var {};

template <> class request_key_var<true> {
protected:
  // should be a pair
  stbox::bytes m_private_key;
  stbox::bytes m_pkey4v;
};
} // namespace internal
} // namespace ypc
