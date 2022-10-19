#pragma once
#include <hpda/common/processor_with_input.h>
#include <hpda/common/processor_with_output.h>

namespace hpda {
namespace processor {
namespace internal {
template <typename InputObjType, typename OutputObjType>
class processor_base
    : public ::hpda::internal::processor_with_output<OutputObjType>,
      public ::hpda::internal::processor_with_input<InputObjType> {
public:
  processor_base(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream)
      : ::hpda::internal::processor_with_input<InputObjType>(upper_stream) {}

  virtual ~processor_base() {}

  typedef ::hpda::internal::processor_with_input<InputObjType> base;

  // inline bool next_input() { return base::next_input(); }

  inline InputObjType input_value() const { return base::input_value(); }
};
} // namespace internal
template <typename InputT, typename OutputT>
using processor_base_t = internal::processor_base<InputT, OutputT>;
} // namespace processor
} // namespace hpda
