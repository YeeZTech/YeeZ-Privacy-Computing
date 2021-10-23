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
} // namespace hpda
