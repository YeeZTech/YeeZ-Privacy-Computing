#pragma once
#include <cstring>
#include <memory>
#include <string>

namespace stbox {

typedef uint8_t byte_t;
class bytes {
public:
  bytes();
  bytes(size_t len);

  bytes(const bytes &v);
  bytes(bytes &&v);
  bytes(std::initializer_list<byte_t> l);
  bytes(const byte_t *v, size_t len);

  bytes &operator=(const bytes &v);
  bytes &operator=(bytes &&v);

  bool operator==(const bytes &v) const;
  bool operator!=(const bytes &v) const;

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

  std::string to_hex() const;
  static bytes from_hex(const std::string &t);

  inline size_t size() const { return m_size; }
  inline const byte_t *value() const {
    if (nullptr != m_value) {
      return m_value.get();
    } else {
      return nullptr;
    }
  }
  inline byte_t *value() {
    if (nullptr != m_value) {
      return m_value.get();
    } else {
      return nullptr;
    }
  }

  inline bool empty() const { return m_size == 0; }

private:
  std::unique_ptr<byte_t[]> m_value;
  size_t m_size;
}; // end class bytes

inline bytes string_to_byte(const std::string &str) {
  return bytes((const byte_t *)(str.c_str()), str.size());
}

inline std::string byte_to_string(const bytes &b) {
  return std::string((const char *)b.value(), b.size());
}

} // namespace stbox

namespace ypc {
inline std::string to_hex(const ::stbox::bytes &b) { return b.to_hex(); }
} // namespace ypc
