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

  virtual bool process() {
    if (!base::has_input_value()) {
      return false;
    }
    auto b = m_func(base::input_value());
    if (b) {
      m_data = base::input_value().make_copy();
    }
    base::consume_input_value();
    return b;
  }

  virtual InputObjType output_value() { return m_data; }

protected:
  typedef std::function<bool(const InputObjType &)> predicate_func_t;
  predicate_func_t m_func;
  InputObjType m_data;
};
} // namespace internal
template <typename... ARGS>
using filter = internal::filter_impl<ntobject<ARGS...>>;

template <typename T> using filter_t = internal::filter_impl<T>;
} // namespace processor
} // namespace hpda
