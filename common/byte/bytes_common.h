#pragma once
#include <cstdint>
#include <stdexcept>

namespace ypc {
namespace utc {
enum class byte_encode { raw_bytes, hex_bytes, base58_bytes, base64_bytes };

namespace internal {
static inline char dec2hex(uint8_t dec) {
  if (dec >= 16) {
    throw std::invalid_argument("Invalid decimal");
  }
  if (dec >= 0 && dec <= 9) {
    return '0' + dec;
  }
  return 'a' + dec - 10;
}

static inline void to_hex(uint8_t num, uint8_t *high, uint8_t *low) {
  *low = dec2hex(num & 0x0f);
  *high = dec2hex(num >> 4);
}

static inline uint8_t hex2dec(uint8_t hex) {
  if (hex >= '0' && hex <= '9')
    return hex - '0';
  if (hex >= 'A' && hex <= 'F')
    return hex - 'A' + 10;
  if (hex >= 'a' && hex <= 'f')
    return hex - 'a' + 10;
  throw std::invalid_argument("Invalid hex");
}
template <typename ByteType1, typename ByteType2>
bool convert_hex_to_bytes(const ByteType1 *s, size_t s_size, ByteType2 *to,
                          size_t len) {
  static_assert(sizeof(ByteType1) == sizeof(ByteType2));
  static_assert(sizeof(ByteType1) == 1);
  if (len < s_size / 2) {
    return false;
  }
  try {
    size_t i = 0;
    while (i * 2 < s_size && i * 2 + 1 < s_size) {
      if (to) {
        to[i] = (hex2dec(s[i * 2]) << 4) + hex2dec(s[i * 2 + 1]);
      }
      i++;
    }
  } catch (std::exception &e) {
    return false;
  }
  return true;
}

template <typename ByteType1, typename ByteType2>
bool convert_bytes_to_hex(const ByteType1 *s, size_t s_size, ByteType2 *to,
                          size_t len) {
  if (len < 2 * s_size) {
    return false;
  }
  for (size_t i = 0; i < s_size; i++) {
    to_hex(s[i], to + 2 * i, to + 2 * i + 1);
  }
  return true;
}

} // namespace internal
} // namespace utc
} // namespace ypc
