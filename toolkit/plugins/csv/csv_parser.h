#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>

namespace ypc {
namespace plugins {
template <typename NT> struct cast_obj_to_package {};
template <typename... ARGS>
struct cast_obj_to_package<::ff::util::ntobject<ARGS...>> {
  typedef ::ff::net::ntpackage<0, ARGS...> type;
};
} // namespace plugins
} // namespace ypc

#ifdef __cplusplus
extern "C" {
#endif
int parse_item_data(const uint8_t *data, int len, void *item);
#ifdef __cplusplus
}
#endif

#define impl_csv_parser(_mtype)                                                \
  int parse_item_data(const uint8_t *data, int len, void *item) {              \
    _mtype *uitem = (_mtype *)item;                                            \
    typedef                                                                    \
        typename ypc::plugins::cast_obj_to_package<_mtype>::type package_t;    \
                                                                               \
    package_t pt;                                                              \
    ff::net::marshaler dm((const char *)data, len,                             \
                          ff::net::marshaler::deseralizer);                    \
    pt.archive(dm);                                                            \
    *uitem = pt;                                                               \
    return 0;                                                                  \
  }
