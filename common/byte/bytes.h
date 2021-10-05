#pragma once
#include "common/byte/base58.h"
#include "common/byte/base64.h"
#include "common/byte/bytes_common.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

namespace ypc {
namespace utc {
namespace internal {
template <typename ByteType> class bytes_base {
public:
  typedef ByteType byte_t;

  static_assert(sizeof(ByteType) == 1,
                "cannot use ByteType that with larger size than 1");

  bytes_base() : m_value(nullptr), m_size(0) {}
  bytes_base(size_t len)
      : m_value(std::unique_ptr<byte_t[]>(new byte_t[len])), m_size(len) {}

  bytes_base(const bytes_base<ByteType> &v) : bytes_base(v.size()) {
    memcpy(m_value.get(), v.m_value.get(), v.size());
    m_size = v.size();
  }

  bytes_base(bytes_base<ByteType> &&v)
      : m_value(std::move(v.m_value)), m_size(v.size()) {}

  bytes_base(std::initializer_list<byte_t> l) {
    if (l.size() > 0) {
      m_value = std::unique_ptr<byte_t[]>(new byte_t[l.size()]);
      std::copy(l.begin(), l.end(), m_value.get());
    }
    m_size = l.size();
  }
  template <typename T> bytes_base(const T *v, size_t len) {
    static_assert(sizeof(T) == 1, "invalid pointer type");
    m_size = len;
    if (len > 0) {
      m_value = std::unique_ptr<byte_t[]>(new byte_t[len]);
      memcpy(m_value.get(), v, len);
    }
  }

  const byte_t &operator[](size_t index) const { return m_value.get()[index]; }
  byte_t &operator[](size_t index) { return m_value.get()[index]; }

  inline size_t size() const { return m_size; }
  inline const byte_t *data() const {
    if (nullptr != m_value) {
      return m_value.get();
    } else {
      return nullptr;
    }
  }
  inline byte_t *data() {
    if (nullptr != m_value) {
      return m_value.get();
    } else {
      return nullptr;
    }
  }

  inline bool empty() const { return m_size == 0; }

protected:
  std::unique_ptr<byte_t[]> m_value;
  size_t m_size;
}; // end class bytes_base

} // namespace internal


template <typename ByteType, byte_encode Format = byte_encode::raw_bytes>
class bytes : public internal::bytes_base<ByteType> {
public:
  typedef ByteType byte_t;
  const static byte_encode format = Format;
  typedef bytes<ByteType, byte_encode::raw_bytes> raw_bytes_t;
  typedef bytes<ByteType, byte_encode::hex_bytes> hex_bytes_t;
  typedef bytes<ByteType, byte_encode::base58_bytes> base58_bytes_t;
  typedef bytes<ByteType, byte_encode::base64_bytes> base64_bytes_t;

  bytes() : internal::bytes_base<ByteType>() {}
  explicit bytes(size_t len) : internal::bytes_base<ByteType>(len) {}
  bytes(const bytes<ByteType, Format> &v) : internal::bytes_base<ByteType>(v) {}
  bytes(bytes<ByteType, Format> &&v) : internal::bytes_base<ByteType>(v) {}
  bytes(std::initializer_list<ByteType> l)
      : internal::bytes_base<ByteType>(l) {}
  template <typename T>
  bytes(const T *v, size_t len) : internal::bytes_base<ByteType>(v, len) {}

  explicit bytes(const char *str)
      : internal::bytes_base<ByteType>(str, strlen(str)) {}
  explicit bytes(const std::string &str)
      : internal::bytes_base<ByteType>(str.data(), str.size()){};

  template <typename BytesType>
  auto as() const -> typename std::enable_if<
      std::is_same<typename BytesType::byte_t, ByteType>::value &&
          (BytesType::format == byte_encode::base58_bytes &&
           Format == byte_encode::raw_bytes),
      BytesType>::type {
    std::string s = encode_base58(
        (const unsigned char *)internal::bytes_base<ByteType>::data(),
        (const unsigned char *)internal::bytes_base<ByteType>::data() +
            internal::bytes_base<ByteType>::size());
    bytes<ByteType, byte_encode::base58_bytes> ret(s.data(), s.size());
    return ret;
  }
  template <typename BytesType>
  auto as() const -> typename std::enable_if<
      std::is_same<typename BytesType::byte_t, ByteType>::value &&
          (BytesType::format == byte_encode::hex_bytes &&
           Format == byte_encode::raw_bytes),
      BytesType>::type {
    bytes<ByteType, byte_encode::hex_bytes> ret(
        internal::bytes_base<ByteType>::size() * 2);
    internal::convert_bytes_to_hex(internal::bytes_base<ByteType>::data(),
                                   internal::bytes_base<ByteType>::size(),
                                   ret.data(), ret.size());
    return ret;
  }

  template <typename BytesType>
  auto as() const -> typename std::enable_if<
      std::is_same<typename BytesType::byte_t, ByteType>::value &&
          (BytesType::format == byte_encode::base64_bytes &&
           Format == byte_encode::raw_bytes),
      BytesType>::type {
    std::string s = base64_encode(
        (unsigned char const *)internal::bytes_base<ByteType>::data(),
        internal::bytes_base<ByteType>::size());
    bytes<ByteType, byte_encode::base64_bytes> ret(s.data(), s.size());
    return ret;
  }

  template <typename BytesType>
  auto as() const -> typename std::enable_if<
      std::is_same<typename BytesType::byte_t, ByteType>::value &&
          (BytesType::format == byte_encode::raw_bytes &&
           Format == byte_encode::hex_bytes),
      BytesType>::type {
    bytes<ByteType, byte_encode::raw_bytes> ret(
        internal::bytes_base<ByteType>::size() / 2);
    auto v = internal::convert_hex_to_bytes(
        internal::bytes_base<ByteType>::data(),
        internal::bytes_base<ByteType>::size(), ret.data(), ret.size());
    if (!v) {
      throw std::invalid_argument("invalid hex string");
    }
    return ret;
  }

  template <typename BytesType>
  auto as() const -> typename std::enable_if<
      std::is_same<typename BytesType::byte_t, ByteType>::value &&
          (BytesType::format == byte_encode::raw_bytes &&
           Format == byte_encode::base58_bytes),
      BytesType>::type {
    std::vector<unsigned char> v;
    auto r = decode_base58(
        std::string((const char *)internal::bytes_base<ByteType>::data(),
                    internal::bytes_base<ByteType>::size()),
        v);
    if (!r) {
      throw std::invalid_argument("Invalid base58 string");
    }
    return bytes<ByteType, byte_encode::raw_bytes>(v.data(), v.size());
  }

  template <typename BytesType>
  auto as() const -> typename std::enable_if<
      std::is_same<typename BytesType::byte_t, ByteType>::value &&
          (BytesType::format == byte_encode::raw_bytes &&
           Format == byte_encode::base64_bytes),
      BytesType>::type {
    std::string s = base64_decode(
        std::string((const char *)internal::bytes_base<ByteType>::data(),
                    internal::bytes_base<ByteType>::size()));
    return bytes<ByteType, byte_encode::raw_bytes>(s.data(), s.size());
  }

  bytes<ByteType, Format> &operator=(const bytes<ByteType, Format> &v) {
    if (&v == this)
      return *this;
    if (v.data()) {
      internal::bytes_base<ByteType>::m_value =
          std::unique_ptr<ByteType[]>(new ByteType[v.size()]);
      memcpy(internal::bytes_base<ByteType>::m_value.get(), v.m_value.get(),
             v.size());
    }
    internal::bytes_base<ByteType>::m_size = v.size();
    return *this;
  }
  bytes<ByteType, Format> &operator=(bytes<ByteType, Format> &&v) {
    internal::bytes_base<ByteType>::m_value = std::move(v.m_value);
    internal::bytes_base<ByteType>::m_size = v.m_size;
    return *this;
  }

  bool operator==(const bytes<ByteType, Format> &v) const {
    if (v.size() != internal::bytes_base<ByteType>::size())
      return false;
    return memcmp(v.data(), internal::bytes_base<ByteType>::data(),
                  internal::bytes_base<ByteType>::size()) == 0;
  }

  bool operator!=(const bytes<ByteType, Format> &v) const {
    return !operator==(v);
  }

  bytes<ByteType, Format> operator+(const bytes<ByteType, Format> &v) const {
    bytes<ByteType, Format> ret(internal::bytes_base<ByteType>::size() +
                                v.size());
    if (internal::bytes_base<ByteType>::size() != 0) {
      memcpy(ret.data(), internal::bytes_base<ByteType>::data(),
             internal::bytes_base<ByteType>::size());
    }
    if (v.size() != 0) {
      memcpy(ret.data() + internal::bytes_base<ByteType>::size(), v.data(),
             v.size());
    }
    return ret;
  }

  bytes<ByteType, Format> operator+(const char *s) const {
    if (!s) {
      return *this;
    }

    bytes<ByteType, Format> ret(internal::bytes_base<ByteType>::size() +
                                strlen(s));
    if (internal::bytes_base<ByteType>::size() != 0) {
      memcpy(ret.data(), internal::bytes_base<ByteType>::data(),
             internal::bytes_base<ByteType>::size());
    }
    memcpy(ret.data() + internal::bytes_base<ByteType>::size(), s, strlen(s));
    return ret;
  }

  bytes<ByteType, Format> operator+(const std::string &s) const {
    if (s.size() == 0) {
      return *this;
    }
    bytes<ByteType, Format> ret(internal::bytes_base<ByteType>::size() +
                                s.size());
    if (internal::bytes_base<ByteType>::size() != 0) {
      memcpy(ret.data(), internal::bytes_base<ByteType>::data(),
             internal::bytes_base<ByteType>::size());
    }
    memcpy(ret.data() + internal::bytes_base<ByteType>::size(), s.data(),
           s.size());
    return ret;
  }
  template <typename T> bytes<ByteType, Format> &operator+=(const T &t) {
    *this = *this + t;
    return *this;
  }
};


} // namespace utc
} // namespace ypc
