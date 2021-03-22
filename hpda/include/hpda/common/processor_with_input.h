#pragma once
#include <hpda/common/common.h>
#include <hpda/common/processor_with_output.h>

namespace hpda {
namespace internal {
template <typename InputObjType> class processor_with_input {
public:
  typedef InputObjType input_type;

  processor_with_input(processor_with_output<InputObjType> *input)
      : m_upper_stream(input){};

  virtual ~processor_with_input() {}

  bool next_input() {
    try {
      return m_upper_stream->next_output();
    } catch (std::exception &e) {
      return false;
    }
  }

  InputObjType input_value() const { return m_upper_stream->output_value(); }

protected:
  internal::processor_with_output<InputObjType> *m_upper_stream;
};

} // namespace internal
} // namespace hpda
