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
                          ff::net::marshaler::serializer);
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
                          ff::net::marshaler::serializer);
    pkg.arch(ld);
  }
  template <typename PackageType>
  static BytesType for_package(const PackageType &_pkg) {
    PackageType &pkg = (PackageType &)_pkg;
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);

    BytesType ret(lm.get_length());
    ff::net::marshaler ld((char *)ret.data(), ret.size(),
                          ff::net::marshaler::serializer);
    pkg.arch(ld);
    return ret;
  }
};

template <typename PackageType> struct make_package {
  template <typename BytesType>
  static PackageType from_bytes(const BytesType &data) {
    PackageType ret;
    ::ff::net::marshaler ar((char *)data.data(), data.size(),
                            ::ff::net::marshaler::deserializer);
    ret.arch(ar);
    return ret;
  }
  template <typename ByteType>
  static PackageType from_bytes(const ByteType *data, uint32_t data_size) {
    PackageType ret;
    ::ff::net::marshaler ar((char *)data, data_size,
                            ::ff::net::marshaler::deserializer);
    ret.arch(ar);
    return ret;
  }
};

template <typename NT, uint32_t PackageID = 0> struct cast_obj_to_package {};
template <uint32_t PackageID, typename... ARGS>
struct cast_obj_to_package<::ff::util::ntobject<ARGS...>, PackageID> {
  typedef ::ff::net::ntpackage<PackageID, ARGS...> type;
};

template <uint32_t PackageID, typename... ARGS>
struct cast_obj_to_package<::ff::net::ntpackage<PackageID, ARGS...>,
                           PackageID> {
  typedef ::ff::net::ntpackage<PackageID, ARGS...> type;
};

template <typename PT> struct cast_package_to_obj {};
template <uint32_t PackageID, typename... ARGS>
struct cast_package_to_obj<::ff::net::ntpackage<PackageID, ARGS...>> {
  typedef ::ff::util::ntobject<ARGS...> type;
};
template <typename... ARGS>
struct cast_package_to_obj<::ff::util::ntobject<ARGS...>> {
  typedef ::ff::util::ntobject<ARGS...> type;
};

} // namespace ypc

namespace ff {
namespace net {
template <typename... ARGS> struct archive_helper<ff::util::ntobject<ARGS...>> {
public:
  typedef ff::util::ntobject<ARGS...> data_t;
  static uint32_t serialize(char *buf, const data_t &d, size_t len) {
    typename ypc::cast_obj_to_package<data_t>::type ret = d;
    marshaler m(buf, len, marshaler::serializer);
    m.archive(ret);
    return m.get_length();
  }
  static uint32_t deserialize(const char *buf, data_t &d, size_t len) {
    typename ypc::cast_obj_to_package<data_t>::type ret;
    marshaler m(buf, len, marshaler::deserializer);
    m.archive(ret);
    d = ret;
    return m.get_length();
  }
  static uint32_t length(const data_t &d) {
    typename ypc::cast_obj_to_package<data_t>::type ret = d;
    marshaler m(marshaler::length_retriver);
    m.archive(ret);
    return m.get_length();
  }
};
} // namespace net
} // namespace ff
