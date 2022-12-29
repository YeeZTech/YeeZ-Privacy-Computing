#pragma once
#include <hpda/processor/processor_base.h>
#include <hpda/processor/set/helper/copy_helper.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename OutputObjType, typename CT, typename KT,
          typename InputObjType>
struct upper_stream_helper {
  template <typename T1, typename T2, typename T3, typename T4>
  static void add_upper_stream(T1 *upper_stream, T2 &upper_streams, T3 &traits,
                               T4 &filler, bool &set_engine, functor *engine) {
    static_assert(
        std::is_same<typename ::ff::util::internal::nt_traits<KT>::type,
                     CT>::value,
        "invalid type");
    upper_streams.push_back(upper_stream);
    traits.push_back([](functor *p) {
      auto *ptr =
          dynamic_cast<::hpda::internal::processor_with_output<InputObjType> *>(
              p);
      return ptr->output_value().template get<KT>();
    });
    filler.push_back([](functor *p, OutputObjType &ret) {
      auto *ptr =
          dynamic_cast<::hpda::internal::processor_with_output<InputObjType> *>(
              p);
      copy_helper<0>::copy(ret, ptr->output_value());
    });

    if (!set_engine) {
      engine->set_engine(upper_stream->get_engine());
      set_engine = true;
    }
    engine->add_predecessor(upper_stream);
  }
};

} // namespace internal
} // namespace processor
} // namespace hpda
