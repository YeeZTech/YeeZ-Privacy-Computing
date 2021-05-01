#pragma once
#include "common/byte.h"
#include <boost/endian/conversion.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ff/network.h>
#include <glog/logging.h>
#include <iostream>
#include <utility>

namespace ypc {

typedef uint8_t byte_t;
using bytes = ::ypc::utc::bytes<byte_t, ::ypc::utc::byte_encode::raw_bytes>;
using hex_bytes = bytes::hex_bytes_t;
using base58_bytes = bytes::base58_bytes_t;
using base64_bytes = bytes::base64_bytes_t;

template <typename T>
auto byte_to_number(byte_t *bytes, size_t len) ->
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type {
  if (len < sizeof(T))
    return T();

  T *val = (T *)bytes;
  T ret = boost::endian::big_to_native(*val);
  return ret;
}

template <typename T>
auto number_to_byte(T val, byte_t *bytes, size_t len) ->
    typename std::enable_if<std::is_arithmetic<T>::value, void>::type {
  if (len < sizeof(T))
    return;

  T v = boost::endian::native_to_big(val);
  T *p = (T *)bytes;
  *p = v;
  return;
}

template <typename T, typename BytesType>
auto byte_to_number(const BytesType &v) ->
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type {
  if (v.size() < sizeof(T))
    return T();

  T *val = (T *)v.data();
  T ret = boost::endian::big_to_native(*val);
  return ret;
}

template <typename BytesType, typename T>
auto number_to_byte(T val) ->
    typename std::enable_if<std::is_arithmetic<T>::value &&
                                utc::is_bytes<BytesType>::value,
                            BytesType>::type {
  T v = boost::endian::native_to_big(val);
  BytesType b((byte_t *)&v, sizeof(v));
  return b;
}

template <typename BytesType, typename T>
auto number_to_byte(T val) ->
    typename std::enable_if<std::is_arithmetic<T>::value &&
                                utc::is_fix_bytes<BytesType>::value,
                            BytesType>::type {
  T v = boost::endian::native_to_big(val);
  BytesType b;
  T *rval = (T *)b.data();
  *rval = v;
  return b;
}
} // namespace ypc

template <typename ByteType, ::ypc::utc::byte_encode Format>
auto operator<<(std::ostream &out,
                const ::ypc::utc::bytes<ByteType, Format> &data) ->
    typename std::enable_if<Format == ::ypc::utc::byte_encode::raw_bytes,
                            std::ostream &>::type {

  typedef typename ::ypc::utc::bytes<ByteType, Format>::hex_bytes_t hex_bytes_t;
  hex_bytes_t hex = data.template as<hex_bytes_t>();
  std::string s((const char *)hex.data(), hex.size());
  out << s;
  return out;
}

template <typename ByteType, ::ypc::utc::byte_encode Format>
auto operator<<(std::ostream &out,
                const ::ypc::utc::bytes<ByteType, Format> &data) ->
    typename std::enable_if<Format != ::ypc::utc::byte_encode::raw_bytes,
                            std::ostream &>::type {

  std::string s((const char *)data.data(), data.size());
  out << s;
  return out;
}

template <typename ByteType, ::ypc::utc::byte_encode Format>
auto operator>>(std::istream &is, ::ypc::utc::bytes<ByteType, Format> &obj) ->
    typename std::enable_if<Format == ::ypc::utc::byte_encode::raw_bytes,
                            std::istream &>::type {
  typedef typename ::ypc::utc::bytes<ByteType, Format>::hex_bytes_t hex_bytes_t;
  std::string s;
  is >> s;
  hex_bytes_t hex(s.c_str(), s.size());
  obj = hex.template as<::ypc::utc::bytes<ByteType, Format>>();
  return is;
}

template <typename ByteType, ::ypc::utc::byte_encode Format>
auto operator>>(std::istream &is, ::ypc::utc::bytes<ByteType, Format> &obj) ->
    typename std::enable_if<Format != ::ypc::utc::byte_encode::raw_bytes,
                            std::istream &>::type {
  std::string s;
  is >> s;
  obj = ::ypc::utc::bytes<ByteType, Format>(s.c_str(), s.size());
  return is;
}


namespace boost {
namespace property_tree {
template <typename String, typename T> struct serialization_translator {};

template <typename String, typename ByteType, ::ypc::utc::byte_encode Format>
struct serialization_translator<String, ::ypc::utc::bytes<ByteType, Format>> {
  typedef String internal_type;
  typedef ::ypc::utc::bytes<ByteType, Format> external_type;

  boost::optional<external_type> get_value(const internal_type &str) {
    std::istringstream stream(str);
    external_type result;
    istream(stream, result);
    if (stream.rdstate() == std::ios::failbit) {
      return boost::none;
    } else {
      return result;
    }
  }

  boost::optional<internal_type> put_value(const external_type &obj) {
    std::ostringstream result;
    ostream(result, obj);
    return result.str();
  }

protected:
  template <typename BT, ::ypc::utc::byte_encode F>
  static auto ostream(std::ostream &out, const ::ypc::utc::bytes<BT, F> &data)
      -> typename std::enable_if<F == ::ypc::utc::byte_encode::raw_bytes,
                                 std::ostream &>::type {

    typedef typename ::ypc::utc::bytes<BT, F>::hex_bytes_t hex_bytes_t;
    hex_bytes_t hex = data.template as<hex_bytes_t>();
    std::string s((const char *)hex.data(), hex.size());
    out << s;
    return out;
  }

  template <typename BT, ::ypc::utc::byte_encode F>
  static auto ostream(std::ostream &out, const ::ypc::utc::bytes<BT, F> &data)
      -> typename std::enable_if<F != ::ypc::utc::byte_encode::raw_bytes,
                                 std::ostream &>::type {

    std::string s((const char *)data.data(), data.size());
    out << s;
    return out;
  }

  template <typename BT, ::ypc::utc::byte_encode F>
  static auto istream(std::istream &is, ::ypc::utc::bytes<BT, F> &obj) ->
      typename std::enable_if<F == ::ypc::utc::byte_encode::raw_bytes,
                              std::istream &>::type {
    typedef typename ::ypc::utc::bytes<BT, F>::hex_bytes_t hex_bytes_t;
    std::string s;
    is >> s;
    hex_bytes_t hex(s.c_str(), s.size());
    obj = hex.template as<::ypc::utc::bytes<BT, F>>();
    return is;
  }

  template <typename BT, ::ypc::utc::byte_encode F>
  static auto istream(std::istream &is, ::ypc::utc::bytes<BT, F> &obj) ->
      typename std::enable_if<F != ::ypc::utc::byte_encode::raw_bytes,
                              std::istream &>::type {
    std::string s;
    is >> s;
    obj = ::ypc::utc::bytes<BT, F>(s.c_str(), s.size());
    return is;
  }
};

typedef std::string string_type;

template <typename ByteType, ::ypc::utc::byte_encode Format>
struct translator_between<string_type, ::ypc::utc::bytes<ByteType, Format>> {
  typedef serialization_translator<string_type,
                                   ::ypc::utc::bytes<ByteType, Format>>
      type;
};

template <> struct translator_between<string_type, string_type> {
  typedef id_translator<string_type> type;
};

} // namespace property_tree
} // namespace boost
