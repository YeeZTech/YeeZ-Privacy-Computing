#pragma once
#include "stbox/ebyte.h"

namespace ypc {
namespace internal {

template <typename ModelT, bool has_model = !std::is_same<ModelT, void>::value>
class model_var {};

template <typename ModelT> class model_var<ModelT, true> {
protected:
  ModelT m_model;
  stbox::bytes m_model_pkey;
  stbox::bytes m_model_hash;
};
} // namespace internal

} // namespace ypc
