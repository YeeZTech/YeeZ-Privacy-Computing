#include "stbox/ebyte.h"

namespace stbox {

bytes::bytes() : m_value(nullptr), m_size(0) {}

bytes::bytes(size_t len)
    : m_value(std::unique_ptr<byte_t[]>(new byte_t[len])), m_size(len) {}

bytes::bytes(const bytes &v) : bytes(v.size()) {
  memcpy(m_value.get(), v.m_value.get(), v.size());
  m_size = v.size();
}

bytes::bytes(bytes &&v) : m_value(std::move(v.m_value)), m_size(v.size()) {}

bytes::bytes(std::initializer_list<byte_t> l) {
  if (l.size() > 0) {
    m_value = std::unique_ptr<byte_t[]>(new byte_t[l.size()]);
    std::copy(l.begin(), l.end(), m_value.get());
  }
  m_size = l.size();
}
bytes::bytes(const byte_t *buf, size_t buf_len) {
  m_size = buf_len;
  if (buf_len > 0) {
    m_value = std::unique_ptr<byte_t[]>(new byte_t[buf_len]);
    memcpy(m_value.get(), buf, buf_len);
  }
}
bytes &bytes::operator=(const bytes &v) {
  if (&v == this)
    return *this;
  if (v.value()) {
    m_value = std::unique_ptr<byte_t[]>(new byte_t[v.size()]);
    memcpy(m_value.get(), v.m_value.get(), v.size());
  }
  m_size = v.size();
  return *this;
}

bytes &bytes::operator=(bytes &&v) {
  m_value = std::move(v.m_value);
  m_size = v.m_size;
  return *this;
}

bool bytes::operator==(const bytes &v) const {
  if (v.size() != size())
    return false;
  return memcmp(v.value(), value(), size()) == 0;
}
bool bytes::operator!=(const bytes &v) const { return !operator==(v); }

byte_t bytes::operator[](size_t index) const { return m_value.get()[index]; }
byte_t &bytes::operator[](size_t index) { return m_value.get()[index]; }

namespace internal {
char dec2hex(uint8_t dec) {
  if (dec >= 0 && dec <= 9) {
    return '0' + dec;
  }
  if (dec < 16) {
    return 'a' + dec - 10;
  }
  throw std::invalid_argument("Invalid decimal");
}
void to_hex(uint8_t num, char *high, char *low) {
  *low = dec2hex(num & 0x0f);
  *high = dec2hex(num >> 4);
}

uint8_t hex2dec(char hex) {
  if (hex >= '0' && hex <= '9')
    return hex - '0';
  if (hex >= 'A' && hex <= 'F')
    return hex - 'A' + 10;
  if (hex >= 'a' && hex <= 'f')
    return hex - 'a' + 10;
  throw std::invalid_argument("Invalid hex");
}
bool convert_hex_to_bytes(const std::string &s, byte_t *buf, size_t &len) {
  try {
    size_t i = 0;
    while (i * 2 < s.size() && i * 2 + 1 < s.size()) {
      if (buf) {
        buf[i] = (hex2dec(s[i * 2]) << 4) + hex2dec(s[i * 2 + 1]);
      }
      i++;
    }
    len = i;
  } catch (std::exception &e) {
    return false;
  }
  return true;
}
} // namespace internal

std::string bytes::to_hex() const {
  std::string s(2 * size(), '0');
  char *p = (char *)s.c_str();
  for (int i = 0; i < size(); i++) {
    internal::to_hex(*(value() + i), p + 2 * i, p + 2 * i + 1);
  }
  return s;
}

bytes bytes::from_hex(const std::string &t) {
  size_t len = 0;
  bool succ = internal::convert_hex_to_bytes(t, nullptr, len);
  if (!succ) {
    throw std::invalid_argument("invalid hex string for from_hex");
  }

  bytes ret(len);
  bool succ_ret = internal::convert_hex_to_bytes(t, ret.value(), len);
  if (!succ_ret) {
    throw std::invalid_argument("invalid hex string for from_hex");
  }

  return ret;
}

} // namespace stbox
