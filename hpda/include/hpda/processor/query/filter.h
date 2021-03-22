#pragma once
#include <hpda/processor/processor_base.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename InputObjType>
class filter_impl : public processor_base<InputObjType, InputObjType> {
public:
  template <typename Func>
  filter_impl(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream,
      Func &&f)
      : processor_base<InputObjType, InputObjType>(upper_stream),
        m_func(std::move(f)) {}

  virtual ~filter_impl() {}

  typedef processor_base<InputObjType, InputObjType> base;

  virtual bool next_output() {
    while (base::next_input()) {
      if (!m_func(base::input_value())) {
        continue;
      }
      return true;
    }
    return false;
  }

  virtual InputObjType output_value() { return base::input_value(); }

protected:
  typedef std::function<bool(const InputObjType &)> predicate_func_t;
  predicate_func_t m_func;
};
} // namespace internal
template <typename... ARGS>
using filter = internal::filter_impl<ntobject<ARGS...>>;
} // namespace processor
} // namespace hpda
