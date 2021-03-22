#include "ypc/byte.h"
#include "ypc/base58.h"
#include "ypc/base64.h"
#include <iomanip>
#include <sstream>

namespace ypc {

namespace internal {
std::string convert_byte_to_hex(const byte_t *buf, size_t len) {
  if (!buf)
    return "";
  std::stringstream s;
  for (size_t i = 0; i < len; i++) {
    s << std::hex << std::setw(2) << std::setfill('0')
      << static_cast<int>(buf[i]);
  }
  return s.str();
}

std::string convert_byte_to_base58(const byte_t *buf, size_t len) {
  return ::ypc::encode_base58(buf, buf + len);
}

std::string convert_byte_to_base64(const byte_t *buf, size_t len) {
  return ::ypc::encode_base64(std::string((const char *)buf, len));
}

bool convert_hex_to_bytes(const std::string &s, byte_t *buf, size_t &len) {
  auto char2int = [](char input) {
    if (input >= '0' && input <= '9')
      return input - '0';
    if (input >= 'A' && input <= 'F')
      return input - 'A' + 10;
    if (input >= 'a' && input <= 'f')
      return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
  };

  try {
    size_t i = 0;
    while (i * 2 < s.size() && i * 2 + 1 < s.size()) {
      if (buf) {
        buf[i] = (char2int(s[i * 2]) << 4) + char2int(s[i * 2 + 1]);
      }
      i++;
    }
    len = i;
  } catch (std::exception &e) {
    return false;
  }
  return true;
}

template <typename CHAR>
void convert_string_to_byte(const CHAR *s, byte_t *buf, size_t size) {
  if (buf) {
    memcpy(buf, s, size);
  }
}

bool convert_base58_to_bytes(const std::string &s, byte_t *buf, size_t &len) {
  std::vector<unsigned char> ret;
  bool rv = ::ypc::decode_base58(s, ret);
  if (rv) {
    len = ret.size();
    convert_string_to_byte(&ret[0], buf, len);
  }

  return rv;
}

bool convert_base64_to_bytes(const std::string &s, byte_t *buf, size_t &len){
  std::string output_string;
  bool rv = ::ypc::decode_base64(s, output_string);

  if (rv) {
    len = output_string.size();
    if (nullptr != buf) {
      convert_string_to_byte(output_string.c_str(), buf, len);
    }
  } else {
    throw std::invalid_argument("Invalid decode_base64.");
  }

  return rv;
}
}
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
bytes::bytes(const std::string &s) {
  m_size = s.size();
  if (m_size > 0) {
    m_value = std::unique_ptr<byte_t[]>(new byte_t[m_size]);
    memcpy(m_value.get(), s.c_str(), m_size);
  }
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

bool bytes::operator<(const bytes &v) const {
  return std::to_string(*this) < std::to_string(v);
}

byte_t bytes::operator[](size_t index) const { return m_value.get()[index]; }
byte_t &bytes::operator[](size_t index) { return m_value.get()[index]; }

std::string bytes::to_base58() const {
  return internal::convert_byte_to_base58(value(), size());
}

std::string bytes::to_base64() const {
  return internal::convert_byte_to_base64(value(), size());
}

std::string bytes::to_hex() const {
  return internal::convert_byte_to_hex(value(), size());
}

bytes bytes::from_base58(const std::string &t) {
  size_t len = 0;
  bool succ = internal::convert_base58_to_bytes(t, nullptr, len);
  if (!succ)
    throw std::invalid_argument("invalid base58 string for from_base58");
  bytes ret(len);
  internal::convert_base58_to_bytes(t, ret.value(), len);
  return ret;
}

bytes bytes::from_base64(const std::string &t) {
  size_t len = 0;
  bool succeed = internal::convert_base64_to_bytes(t, nullptr, len);
  if (!succeed) {
    throw std::invalid_argument("invalid base64 string for from_base64");
  }

  bytes ret(len);
  internal::convert_base64_to_bytes(t, ret.value(), len);

  return ret;
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

bytes &bytes::append_bytes(const byte_t *buf, size_t buf_len) {
  if (buf_len <= 0) {
    return *this;
  }

  auto new_value = std::unique_ptr<byte_t[]>(new byte_t[m_size + buf_len]);
  memcpy(new_value.get(), m_value.get(), m_size);
  memcpy(new_value.get() + m_size, buf, buf_len);

  m_value = std::move(new_value);
  m_size += buf_len;
  return *this;
}
bytes &bytes::append_bytes(const bytes &v) {
  return append_bytes(v.value(), v.size());
}

std::string encode_hex(const std::string &bytes_str) {
  auto bytes = string_to_byte(bytes_str);
  return bytes.to_hex();
}
} // namespace ypc
