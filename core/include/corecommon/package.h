#pragma once
#include <ff/net/common/archive.h>
#include <ff/net/middleware/ntpackage.h>

namespace ypc {

template <typename BytesType> struct make_bytes {

  template <typename PackageType, typename... ARGS, typename... PARGS>
  static BytesType for_package(PARGS... params) {
    PackageType p;
    p.template set<ARGS...>(params...);
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    p.arch(lm);

    BytesType ret(lm.get_length());
    ff::net::marshaler ld((char *)ret.data(), ret.size(),
                          ff::net::marshaler::seralizer);
    p.arch(ld);
    return ret;
  }

  template <typename ByteType, typename PackageType>
  static void for_package(ByteType *data, uint32_t data_size,
                          PackageType &pkg) {
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);

    BytesType ret(lm.get_length());
    ff::net::marshaler ld((char *)data, data_size,
                          ff::net::marshaler::seralizer);
    pkg.arch(ld);
  }
};

template <typename PackageType> struct make_package {
  template <typename BytesType>
  static PackageType from_bytes(const BytesType &data) {
    PackageType ret;
    ::ff::net::marshaler ar((char *)data.data(), data.size(),
                            ::ff::net::marshaler::deseralizer);
    ret.arch(ar);
    return ret;
  }
  template <typename ByteType>
  static PackageType from_bytes(const ByteType *data, uint32_t data_size) {
    PackageType ret;
    ::ff::net::marshaler ar((char *)data, data_size,
                            ::ff::net::marshaler::deseralizer);
    ret.arch(ar);
    return ret;
  }
};
} // namespace ypc
