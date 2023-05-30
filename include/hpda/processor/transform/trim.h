#pragma once
#include <hpda/processor/processor_base.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename InputObjType, typename OutputObjType>
class trim_impl : public processor_base<InputObjType, OutputObjType> {
public:
  trim_impl(::hpda::internal::processor_with_output<InputObjType> *upper_stream)
      : processor_base<InputObjType, OutputObjType>(upper_stream) {}

  virtual ~trim_impl() {}

  typedef processor_base<InputObjType, OutputObjType> base;

  virtual bool process() {
    if (!base::has_input_value()) {
      return false;
    }
    m_data = base::input_value().make_copy();
    base::consume_input_value();
    return true;
  }

  virtual OutputObjType output_value() { return m_data; }

protected:
  OutputObjType m_data;
};
} // namespace internal
template <typename TI, typename TO> using trim_t = internal::trim_impl<TI, TO>;
} // namespace processor
} // namespace hpda
