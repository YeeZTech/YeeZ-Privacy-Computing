#pragma once
#include <hpda/common/stream_policy.h>
#include <hpda/output/output_base.h>

namespace hpda {
namespace output {
namespace internal {
template <typename InputObjType>
class memory_output_impl : public output_base<InputObjType> {
public:
  memory_output_impl(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream)
      : output_base<InputObjType>(upper_stream) {}

  virtual ~memory_output_impl() {}

  typedef output_base<InputObjType> base;

  virtual bool process() {
    if (!base::has_input_value()) {
      return false;
    }
    m_data.push_back(output_base<InputObjType>::input_value().make_copy());
    base::consume_input_value();
    return true;
  }

  std::vector<InputObjType> &values() { return m_data; }
  const std::vector<InputObjType> &values() const { return m_data; }

protected:
  std::vector<InputObjType> m_data;
};
} // namespace internal

template <typename... ARGS>
using memory_output = internal::memory_output_impl<ntobject<ARGS...>>;
template <typename T> using memory_output_t = internal::memory_output_impl<T>;
} // namespace output
} // namespace hpda
