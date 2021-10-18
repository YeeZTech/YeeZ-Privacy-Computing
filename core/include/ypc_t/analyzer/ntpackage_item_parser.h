#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>

namespace ypc {
template <typename ByteType, typename UserItemT> struct ntpackage_item_parser {

  template <typename NT> struct cast_obj_to_package {};
  template <typename... ARGS>
  struct cast_obj_to_package<::ff::util::ntobject<ARGS...>> {
    typedef ::ff::net::ntpackage<0, ARGS...> type;
  };
  static UserItemT parser(const ByteType *data, size_t len) {
    typedef typename cast_obj_to_package<UserItemT>::type package_t;

    package_t pt;
    ff::net::marshaler dm((const char *)data, len,
                          ff::net::marshaler::deseralizer);
    pt.archive(dm);
    UserItemT ret = pt;
    return ret;
  }
};
} // namespace ypc
