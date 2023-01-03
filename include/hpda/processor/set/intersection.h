#pragma once
#include <hpda/processor/processor_base.h>
#include <hpda/processor/set/helper/copy_helper.h>
#include <hpda/processor/set/helper/upper_stream_helper.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename OutputObjType, typename CT>
class ordered_intersection_impl
    : public ::hpda::internal::processor_with_output<OutputObjType> {

public:
  ordered_intersection_impl() : m_set_engine(false) {}

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

  bool has_intersection() {
    CT val = m_traits[0](m_upper_streams[0]);
    for (size_t i = 1; i < m_upper_streams.size(); i++) {
      CT tmp = m_traits[i](m_upper_streams[i]);
      if (val != tmp) {
        return false;
      }
    }
    return true;
  }

  void next_value() {
    CT val = m_traits[0](m_upper_streams[0]);
    for (size_t i = 1; i < m_upper_streams.size(); i++) {
      CT tmp = m_traits[i](m_upper_streams[i]);
      if (val > tmp) {
        val = tmp;
      }
    }
    for (size_t i = 0; i < m_upper_streams.size(); i++) {
      CT tmp = m_traits[i](m_upper_streams[i]);
      if (val == tmp) {
        m_upper_streams[i]->reset_done_value();
      }
    }
  }

  void fill_output() {
    for (size_t i = 0; i < m_upper_streams.size(); i++) {
      m_filler[i](m_upper_streams[i], m_data);
    }
  }

  virtual bool process() {
    if (m_upper_streams.size() < 2) {
      return false;
    }
    for (auto &s : m_upper_streams) {
      bool has_value = s->has_value();
      if (!has_value) {
        return false;
      }
    }

    bool ret = has_intersection();
    if (ret) {
      fill_output();
    }
    next_value();
    return ret;
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
using ordered_intersection =
    internal::ordered_intersection_impl<ntobject<ARGS...>, CT>;
} // namespace processor
} // namespace hpda
