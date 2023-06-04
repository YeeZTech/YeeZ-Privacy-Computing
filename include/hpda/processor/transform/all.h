#pragma once
#include <hpda/processor/processor_base.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename InputObjType>
class all_impl : public processor_base<InputObjType, InputObjType> {
public:
  all_impl(::hpda::internal::processor_with_output<InputObjType> *upper_stream)
      : processor_base<InputObjType, InputObjType>(upper_stream) {}

  virtual ~all_impl() {}

  typedef processor_base<InputObjType, InputObjType> base;

  virtual bool process() {
    if (!base::has_input_value()) {
      return false;
    }
    m_data.push_back(base::input_value().make_copy());
    base::consume_input_value();
    return true;
  }

  virtual InputObjType output_value() { return m_data.back(); }

  std::vector<InputObjType> &values() { return m_data; }
  const std::vector<InputObjType> &values() const { return m_data; }

protected:
  std::vector<InputObjType> m_data;
};
} // namespace internal
template <typename T> using all_t = internal::all_impl<T>;
} // namespace processor
} // namespace hpda
