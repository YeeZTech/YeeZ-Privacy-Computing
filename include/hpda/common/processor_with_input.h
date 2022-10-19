#pragma once
#include <hpda/common/common.h>
#include <hpda/common/processor_with_output.h>
#include <hpda/engine/engine.h>
#include <hpda/engine/functor.h>

namespace hpda {
namespace internal {
template <typename InputObjType>
class processor_with_input : virtual public functor {
public:
  typedef InputObjType input_type;

  processor_with_input(processor_with_output<InputObjType> *input)
      : functor(), m_upper_stream(input) {
    functor::set_engine(input->get_engine());
    add_predecessor(input);
  };

  virtual ~processor_with_input() {}

  // virtual bool next_input() {
  // try {
  // return m_upper_stream->next_output();
  //} catch (std::exception &e) {
  // return false;
  //}
  //}

  InputObjType input_value() const { return m_upper_stream->output_value(); }

  inline bool has_input_value() const { return m_upper_stream->has_value(); }

  inline void consume_input_value() { m_upper_stream->reset_done_value(); }

  inline void change_upper_stream(processor_with_output<InputObjType> *input) {
    m_upper_stream = input;
  }

protected:
  internal::processor_with_output<InputObjType> *m_upper_stream;
};

} // namespace internal
template <typename... ARGS>
using processor_with_input = internal::processor_with_input<ntobject<ARGS...>>;
template <typename T>
using processor_with_input_t = internal::processor_with_input<T>;
} // namespace hpda
