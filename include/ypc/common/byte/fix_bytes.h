//! No use for now, so don't use this for now.
#pragma once
#include <cstdint>
#include <utility>

namespace ypc {
namespace utc {

#if __cplusplus >= 201402L
namespace internal {
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
#endif

template <typename ByteType, size_t ByteLength = 32> class fix_bytes {
public:
  typedef ByteType byte_t;
  const static size_t fixed_length;

  static_assert(sizeof(ByteType) == 1,
                "cannot use ByteType that with larger size than 1");

  fix_bytes() : m_value{0} {};
#if __cplusplus >= 201402L
  fix_bytes(std::initializer_list<byte_t> l)
      : m_value(internal::make_array<byte_t, ByteLength>(l)) {}
#endif
  fix_bytes(const fix_bytes<ByteType, ByteLength> &v) : m_value(v.m_value) {}
  fix_bytes(fix_bytes &&v) : m_value(std::move(v.m_value)) {}

  template <typename T> fix_bytes(const T *buf, size_t buf_len) {
    static_assert(sizeof(T) == sizeof(ByteType), "invalid buf pointer");
    if (buf_len >= ByteLength) {
      memcpy(m_value.data(), buf, ByteLength);
    } else {
      throw std::runtime_error("unmatched byte length");
    }
  }

  fix_bytes<ByteType, ByteLength> &
  operator=(const fix_bytes<ByteType, ByteLength> &v) {
    if (&v == this)
      return *this;
    m_value = v.m_value;
    return *this;
  }

  fix_bytes<ByteType, ByteLength> &
  operator=(fix_bytes<ByteType, ByteLength> &&v) {
    m_value = std::move(v.m_value);
    return *this;
  }

  bool operator==(const fix_bytes<ByteType, ByteLength> &v) const {
    return m_value == v.m_value;
  }

  bool operator!=(const fix_bytes<ByteType, ByteLength> &v) const {
    return m_value != v.m_value;
  }
  bool operator<(const fix_bytes<ByteType, ByteLength> &v) const {
    return memcmp(m_value, v.m_value, ByteLength) < 0;
  }
  template <size_t BL>
  fix_bytes<ByteType, BL + ByteLength>
  operator+(const fix_bytes<ByteType, BL> &v) const {
    fix_bytes<ByteType, BL + ByteLength> ret;
    std::copy(ret.m_value.begin(), ret.m_value.begin() + ByteLength,
              m_value.begin());
    std::copy(ret.m_value.begin() + ByteLength, ret.m_value.end(),
              v.m_value.begin());
    return ret;
  }
  const byte_t &operator[](size_t index) const { return m_value[index]; }
  byte_t &operator[](size_t index) { return m_value[index]; }

  inline size_t size() const { return ByteLength; }

  inline const byte_t *value() const { return m_value.data(); }

  inline byte_t *value() { return m_value.data(); }

  bool empty() const { return m_value.empty(); }

protected:
  std::array<byte_t, ByteLength> m_value;
}; // end class fix_bytes
} // namespace utc
} // namespace ypc
