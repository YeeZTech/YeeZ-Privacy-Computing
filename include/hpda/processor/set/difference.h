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
    CT val_0 = m_traits[0](m_upper_streams[0]);
    CT val_min = val_0;
    for (size_t i = 1; i < m_upper_streams.size(); i++) {
      if (m_upper_streams[i]->has_value()) {
        CT val_i = m_traits[i](m_upper_streams[i]);
        val_min = std::min(val_min, val_i);
      }
    }
    bool flag_min = false;
    for (size_t i = 1; i < m_upper_streams.size(); i++) {
      if (m_upper_streams[i]->has_value()) {
        CT val_i = m_traits[i](m_upper_streams[i]);
        if (val_i == val_min) {
          flag_min = true;
          m_upper_streams[i]->reset_done_value();
        }
      }
    }
    if (flag_min) {
      if (val_0 == val_min) {
        m_upper_streams[0]->reset_done_value();
      }
      return false;
    }

    m_filler[0](m_upper_streams[0], m_data);
    m_upper_streams[0]->reset_done_value();
    return true;
  }

  virtual bool process() {
    if (m_upper_streams.size() < 2) {
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
