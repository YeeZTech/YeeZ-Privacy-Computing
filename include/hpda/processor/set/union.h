#pragma once
#include <hpda/processor/processor_base.h>
#include <hpda/processor/set/helper/copy_helper.h>
#include <hpda/processor/set/helper/upper_stream_helper.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename OutputObjType, typename CT>
class ordered_union_impl
    : public ::hpda::internal::processor_with_output<OutputObjType> {

public:
  ordered_union_impl() : m_set_engine(false) {}

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

  bool has_union() {
    bool flag = false;
    size_t index;
    CT val;
    for (size_t i = 0; i < m_upper_streams.size(); i++) {
      if (m_upper_streams[i]->has_value()) {
        CT tmp = m_traits[i](m_upper_streams[i]);
        if (!flag) {
          val = tmp;
          index = i;
          flag = true;
        }
        if (val > tmp) {
          val = tmp;
          index = i;
        }
      }
    }
    if (!flag) {
      return false;
    }
    for (size_t i = 0; i < m_upper_streams.size(); i++) {
      if (m_upper_streams[i]->has_value()) {
        CT tmp = m_traits[i](m_upper_streams[i]);
        if (i != index && tmp == val) {
          m_upper_streams[i]->reset_done_value();
        }
      }
    }
    m_filler[index](m_upper_streams[index], m_data);
    m_upper_streams[index]->reset_done_value();
    return true;
  }

  virtual bool process() {
    if (m_upper_streams.size() < 2) {
      return false;
    }
    return has_union();
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
using ordered_union = internal::ordered_union_impl<ntobject<ARGS...>, CT>;
} // namespace processor
} // namespace hpda
