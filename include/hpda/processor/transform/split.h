#pragma once
#include <hpda/extractor/raw_data.h>
#include <hpda/output/output_base.h>
//#include <hpda/processor/processor_base.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename InputObjType>
class split_impl : public ::hpda::output::internal::output_base<InputObjType> {
public:
  typedef ::hpda::output::internal::output_base<InputObjType> base;

  typedef ::hpda::extractor::internal::raw_data_impl<InputObjType> stream_type;
  split_impl(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream)
      : ::hpda::output::internal::output_base<InputObjType>(upper_stream) {
  }

  ::hpda::internal::processor_with_output<InputObjType> *new_split_stream() {
    m_streams.push_back(std::unique_ptr<stream_type>(new stream_type()));
    auto ret = m_streams.back().get();
    ret->set_engine(base::get_engine());
    ret->add_predecessor(this);
    return ret;
  }

  virtual bool process() {
    if (!base::has_input_value()) {
      return false;
    }
    auto t = base::input_value();
    for (auto &it : m_streams) {
      it->add_data(t.make_copy());
    }
    base::consume_input_value();
    return true;
  }

protected:
  std::vector<std::unique_ptr<stream_type>> m_streams;
};
} // namespace internal
template <typename... ARGS>
using split = internal::split_impl<ntobject<ARGS...>>;
} // namespace processor
} // namespace hpda
