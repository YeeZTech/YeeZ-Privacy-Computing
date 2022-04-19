#pragma once
#include <hpda/common/common.h>
#include <hpda/engine/functor.h>

namespace hpda {
namespace internal {

template <typename OutputObjType>
class processor_with_output : virtual public functor {
public:
  typedef OutputObjType output_type;
  virtual ~processor_with_output() {}
  virtual OutputObjType output_value() = 0;
};

} // namespace internal
template <typename... ARGS>
using processor_with_output =
    internal::processor_with_output<ntobject<ARGS...>>;
template <typename T>
using processor_with_output_t = internal::processor_with_output<T>;
} // namespace hpda
