#pragma once
#include "common.h"
#include <boost/endian/conversion.hpp>
#include <ff/network.h>
#include <glog/logging.h>
#include <utility>

namespace ypc {

typedef uint8_t byte_t;
namespace internal {
std::string convert_byte_to_hex(const byte_t *buf, size_t len);
std::string convert_byte_to_base58(const byte_t *buf, size_t len);
std::string convert_byte_to_base64(const byte_t *buf, size_t len);

bool convert_hex_to_bytes(const std::string &s, byte_t *buf, size_t &len);
bool convert_base58_to_bytes(const std::string &s, byte_t *buf, size_t &len);
bool convert_base64_to_bytes(const std::string &s, byte_t *buf, size_t &len);

template <typename T, std::size_t N, std::size_t... Ns>
std::array<T, N> make_array_impl(std::initializer_list<T> t,
                                 std::index_sequence<Ns...>) {
  return std::array<T, N>{*(t.begin() + Ns)...};
}

template <typename T, std::size_t N>
std::array<T, N> make_array(std::initializer_list<T> t) {
  if (N < t.size())
    throw std::out_of_range("make_array out of range");
  return make_array_impl<T, N>(t, std::make_index_sequence<N>());
};
} // end namespace internal

template <size_t ByteLength = 32> class fix_bytes {
public:
  fix_bytes() : m_value{0} {};
  fix_bytes(std::initializer_list<byte_t> l)
      : m_value(internal::make_array<byte_t, ByteLength>(l)) {}
  fix_bytes(const fix_bytes<ByteLength> &v) : m_value(v.m_value) {}
  fix_bytes(fix_bytes &&v) : m_value(std::move(v.m_value)) {}
  fix_bytes(const byte_t *buf, size_t buf_len) {
    if (buf_len >= ByteLength) {
      memcpy(m_value.data(), buf, ByteLength);
    } else {
      LOG(ERROR) << "buf len is: " << buf_len << ", less than " << ByteLength;
    }
  }

  fix_bytes<ByteLength> &operator=(const fix_bytes<ByteLength> &v) {
    if (&v == this)
      return *this;
    m_value = v.m_value;
    return *this;
  }

  fix_bytes<ByteLength> &operator=(fix_bytes<ByteLength> &&v) {
    m_value = std::move(v.m_value);
    return *this;
  }

  bool operator==(const fix_bytes<ByteLength> &v) const {
    return m_value == v.m_value;
  }

  bool operator!=(const fix_bytes<ByteLength> &v) const {
    return m_value != v.m_value;
  }
  bool operator<(const fix_bytes<ByteLength> &v) const {
    return memcmp(m_value, v.m_value, ByteLength) < 0;
  }
  template <size_t BL>
  fix_bytes<BL + ByteLength> operator+(const fix_bytes<BL> &v) const {
    fix_bytes<BL + ByteLength> ret;
    std::copy(ret.m_value.begin(), ret.m_value.begin() + ByteLength,
              m_value.begin());
    std::copy(ret.m_value.begin() + ByteLength, ret.m_value.end(),
              v.m_value.begin());
    return ret;
  }
  byte_t operator[](size_t index) const { return m_value[index]; }
  byte_t &operator[](size_t index) { return m_value[index]; }

  std::string to_base58() const {
    return internal::convert_byte_to_base58(value(), size());
  }

  std::string to_base64() const {
    return internal::convert_byte_to_base64(value(), size());
  }

  std::string to_hex() const {
    return internal::convert_byte_to_hex(value(), size());
  }

  inline size_t size() const { return ByteLength; }

  inline const byte_t *value() const { return m_value.data(); }

  inline byte_t *value() { return m_value.data(); }

  static fix_bytes<ByteLength> from_base58(const std::string &t) {
    fix_bytes<ByteLength> ret;
    size_t s = ret.size();
    bool succ = internal::convert_base58_to_bytes(t, ret.value(), s);
    if (!succ)
      throw std::invalid_argument("invalid base58 string for from_base58");
    return ret;
  }

  static fix_bytes<ByteLength> from_base64(const std::string &t) {
    fix_bytes<ByteLength> ret;
    size_t s = ret.size();
    bool succ = internal::convert_base64_to_bytes(t, ret.value(), s);
    if (!succ)
      throw std::invalid_argument("invalid base64 string for from_base64");
    return ret;
  }

  static fix_bytes<ByteLength> from_hex(const std::string &t) {
    fix_bytes<ByteLength> ret;
    size_t s = ret.size();
    bool succ = internal::convert_hex_to_bytes(t, ret.value(), s);
    if (!succ)
      throw std::invalid_argument("invalid hex string for from_hex");
    return ret;
  }

  bool empty() const { return m_value.empty(); }

protected:
  std::array<byte_t, ByteLength> m_value;
}; // end class fix_bytes

template <typename T> struct is_fix_bytes { const static bool value = false; };
template <size_t N> struct is_fix_bytes<fix_bytes<N>> {
  const static bool value = true;
};

class bytes {
public:
  bytes();
  bytes(size_t len);

  bytes(const bytes &v);
  bytes(bytes &&v);
  bytes(std::initializer_list<byte_t> l);
  bytes(const std::string &s);
  bytes(const byte_t *v, size_t len);

  bytes &operator=(const bytes &v);
  bytes &operator=(bytes &&v);

  bool operator==(const bytes &v) const;
  bool operator!=(const bytes &v) const;
  bool operator<(const bytes &v) const;

  bytes operator+(const bytes &v) const {
    bytes ret(size() + v.size());
    if (size() != 0) {
      memcpy(ret.value(), value(), size());
    }
    if (v.size() != 0) {
      memcpy(ret.value() + size(), v.value(), v.size());
    }
    return ret;
  }
  bytes operator+(const std::string &v) const {
    bytes ret(size() + v.size());
    if (size() != 0) {
      memcpy(ret.value(), value(), size());
    }
    if (v.size() != 0) {
      memcpy(ret.value() + size(), v.data(), v.size());
    }
    return ret;
  }

  byte_t operator[](size_t index) const;
  byte_t &operator[](size_t index);

  std::string to_base58() const;
  std::string to_base64() const;
  std::string to_hex() const;

  static bytes from_base58(const std::string &t);
  static bytes from_base64(const std::string &t);
  static bytes from_hex(const std::string &t);

  inline size_t size() const { return m_size; }
  inline const byte_t *value() const {
    if(nullptr != m_value){
      return m_value.get();
    }else{
      return nullptr;
    }
  }
  inline byte_t *value() {
    if(nullptr != m_value){
      return m_value.get();
    }else{
      return nullptr;
    }
  }

  inline bool empty() const { return m_size == 0; }

  bytes &append_bytes(const byte_t *buf, size_t buf_len);
  bytes &append_bytes(const bytes &v);

private:
  std::unique_ptr<byte_t[]> m_value;
  size_t m_size;
}; // end class bytes

inline bytes string_to_byte(const std::string &str) {
  return bytes((const byte_t *)(str.c_str()), str.size());
}

inline std::string byte_to_string(const bytes &b) {
  return std::string((const char*)b.value(), b.size());
}

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

  T *val = (T *)v.value();
  T ret = boost::endian::big_to_native(*val);
  return ret;
}

template <typename BytesType, typename T>
auto number_to_byte(T val) ->
    typename std::enable_if<std::is_arithmetic<T>::value &&
                                std::is_same<BytesType, bytes>::value,
                            BytesType>::type {
  T v = boost::endian::native_to_big(val);
  BytesType b((byte_t *)&v, sizeof(v));
  return b;
}

template <typename BytesType, typename T>
auto number_to_byte(T val) ->
    typename std::enable_if<std::is_arithmetic<T>::value &&
                                is_fix_bytes<BytesType>::value,
                            BytesType>::type {
  T v = boost::endian::native_to_big(val);
  BytesType b;
  T *rval = (T *)b.value();
  *rval = v;
  return b;
}
template <typename FixBytesType> FixBytesType to_fix_bytes(const bytes &val) {
  FixBytesType ret;
  assert(val.size() == ret.size());
  memcpy(ret.value(), val.value(), ret.size());
  return ret;
}

template <typename FixBytesType> bytes from_fix_bytes(const FixBytesType &val) {
  return bytes(val.value(), val.size());
}

std::string encode_hex(const std::string &bytes_str);
} // namespace ypc
namespace std {
template <> struct hash<::ypc::bytes> {
  typedef ::ypc::bytes argument_type;
  typedef std::size_t result_type;
  result_type operator()(argument_type const &s) const noexcept {
    return std::hash<std::string>{}(byte_to_string(s));
  }
};

template <size_t ByteLength> struct hash<::ypc::fix_bytes<ByteLength>> {
  typedef ::ypc::fix_bytes<ByteLength> argument_type;
  typedef std::size_t result_type;
  result_type operator()(argument_type const &s) const noexcept {
    return std::hash<std::string>{}(byte_to_string(s));
  }
};

inline std::string to_string(const ::ypc::bytes &s) {
  return byte_to_string(s);
}
} // namespace std

namespace ypc {
inline std::string to_hex(const bytes &b) { return b.to_hex(); }
} // namespace ypc

