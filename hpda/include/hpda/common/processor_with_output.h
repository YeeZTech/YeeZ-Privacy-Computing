#pragma once
#include <hpda/common/common.h>

namespace hpda {
namespace internal {

template <typename OutputObjType> class processor_with_output {
public:
  typedef OutputObjType output_type;
  virtual ~processor_with_output() {}
  virtual bool next_output() = 0;
  virtual OutputObjType output_value() = 0;
};

} // namespace internal
} // namespace hpda
