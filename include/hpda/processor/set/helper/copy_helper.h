#pragma once
#include <ff/util/type_list.h>

namespace hpda {
namespace processor {
namespace internal {

template <int Index> struct copy_helper {
  // TODO optimize for rvalue(&&)
  template <typename T1, typename T2>
  static auto copy(T1 &target, T2 &&source) -> typename std::enable_if<
      (std::remove_reference<T2>::type::type_list::len > Index), void>::type {
    typedef typename ::ff::util::get_type_at_index_in_typelist<
        typename std::remove_reference<T2>::type::type_list, Index>::type
        c_type;
    target.template set<c_type>(source.template get<c_type>());
    copy_helper<Index + 1>::copy(target, std::forward<T2>(source));
  }

  template <typename T1, typename T2>
  static auto copy(T1 &target, const T2 &source) -> typename std::enable_if<
      (std::remove_reference<T2>::type::type_list::len <= Index), void>::type {}
};

} // namespace internal
} // namespace processor
} // namespace hpda
