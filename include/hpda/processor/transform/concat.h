#pragma once
#include <hpda/processor/processor_base.h>

namespace hpda {
namespace processor {
namespace internal {
template <typename InputObjType>
class concat_impl : public processor_base<InputObjType, InputObjType> {
public:
  typedef processor_base<InputObjType, InputObjType> base;
  concat_impl(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream)
      : processor_base<InputObjType, InputObjType>(upper_stream) {
    m_upper_streams.push_back(upper_stream);
    m_index = 0;
  }

  void add_upper_stream(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream) {
    m_upper_streams.push_back(upper_stream);
    base::add_predecessor(upper_stream);
  }
  virtual bool process() {
    while (m_index < m_upper_streams.size() &&
           !m_upper_streams[m_index]->has_value()) {
      m_index++;
    }
    if (m_index >= m_upper_streams.size()) {
      return false;
    }
    auto b = m_upper_streams[m_index]->has_value();
    if (b) {
      m_data = m_upper_streams[m_index]->output_value().make_copy();
    }
    m_upper_streams[m_index]->reset_done_value();
    return b;
  }

  virtual InputObjType output_value() { return m_data; }

protected:
  typedef ::hpda::internal::processor_with_output<InputObjType> upper_stream_t;
  size_t m_index;
  std::vector<upper_stream_t *> m_upper_streams;
  InputObjType m_data;
};
} // namespace internal
template <typename... ARGS>
using concat = internal::concat_impl<ntobject<ARGS...>>;
} // namespace processor
} // namespace hpda
