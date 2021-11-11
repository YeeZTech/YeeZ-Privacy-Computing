#pragma once
#include <hpda/common/processor_with_input.h>
#include <hpda/common/processor_with_output.h>

namespace hpda {
namespace output {
namespace internal {

template <typename InputObjType>
class output_base
    : public ::hpda::internal::processor_with_input<InputObjType> {
public:
  output_base(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream)
      : ::hpda::internal::processor_with_input<InputObjType>(upper_stream) {}

  virtual ~output_base() {}

  typedef ::hpda::internal::processor_with_input<InputObjType> base;

  // inline bool next_input() { return base::next_input(); }

  InputObjType input_value() const { return base::input_value(); }

  virtual void done_value() { functor::m_has_value = false; };
};
} // namespace internal
} // namespace output
} // namespace hpda
