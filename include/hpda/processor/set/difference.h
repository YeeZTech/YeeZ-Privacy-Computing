#pragma once
#include <hpda/processor/processor_base.h>
#include <hpda/processor/set/helper/copy_helper.h>
#include <hpda/processor/set/helper/upper_stream_helper.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename OutputObjType, typename CT>
class ordered_difference_impl
    : public ::hpda::internal::processor_with_output<OutputObjType> {

public:
  ordered_difference_impl() : m_set_engine(false) {}

  template <typename KT, typename InputObjType>
  void add_upper_stream(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream) {
    return upper_stream_helper<OutputObjType, CT, KT,
                               InputObjType>::add_upper_stream(upper_stream,
                                                               m_upper_streams,
                                                               m_traits,
                                                               m_filler,
                                                               m_set_engine,
                                                               this);
  }

  bool has_difference() {
    if (!m_upper_streams[1]->has_value()) {
      m_filler[0](m_upper_streams[0], m_data);
      m_upper_streams[0]->reset_done_value();
      return true;
    }
    CT val0 = m_traits[0](m_upper_streams[0]);
    CT val1 = m_traits[1](m_upper_streams[1]);
    if (val0 < val1) {
      m_filler[0](m_upper_streams[0], m_data);
      m_upper_streams[0]->reset_done_value();
      return true;
    } else if (val0 > val1) {
      m_upper_streams[1]->reset_done_value();
      return false;
    }
    m_upper_streams[0]->reset_done_value();
    m_upper_streams[1]->reset_done_value();
    return false;
  }

  virtual bool process() {
    if (m_upper_streams.size() != 2) {
      return false;
    }
    if (!m_upper_streams[0]->has_value()) {
      return false;
    }
    return has_difference();
  }

  virtual OutputObjType output_value() { return m_data; }

protected:
  std::vector<functor *> m_upper_streams;
  typedef std::function<CT(functor *)> kt_traits_t;
  std::vector<kt_traits_t> m_traits;
  bool m_set_engine;

  typedef std::function<void(functor *, OutputObjType &)> output_t;
  std::vector<output_t> m_filler;
  OutputObjType m_data;
};

} // namespace internal
template <typename CT, typename... ARGS>
using ordered_difference =
    internal::ordered_difference_impl<ntobject<ARGS...>, CT>;
} // namespace processor
} // namespace hpda
