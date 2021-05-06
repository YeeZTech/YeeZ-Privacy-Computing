#pragma once
#include "common/byte/bytes.h"
#include "common/byte/fix_bytes.h"
#include "ff/net/common/archive.h"

namespace ypc {
namespace utc {
template <typename T> struct is_fix_bytes { const static bool value = false; };
template <typename ByteType, size_t N>
struct is_fix_bytes<fix_bytes<ByteType, N>> {
  const static bool value = true;
};

template <typename T> struct is_bytes { const static bool value = false; };

template <typename ByteType, byte_encode Format>
struct is_bytes<bytes<ByteType, Format>> {
  const static bool value = true;
};
} // namespace utc
} // namespace ypc

namespace ff {
namespace net {
template <typename ByteType, ::ypc::utc::byte_encode Format>
class archive_helper<::ypc::utc::bytes<ByteType, Format>> {
public:
  typedef ::ypc::utc::bytes<ByteType, Format> data_t;
  static uint32_t serialize(char *buf, const data_t &d, size_t len) {
    size_t s = d.size();
    memcpy(buf, (const char *)&s, sizeof(s));
    memcpy(buf + sizeof(s), (const char *)d.data(), s);
    return s + sizeof(s);
  }
  static uint32_t deserialize(const char *buf, data_t &d, size_t len) {
    size_t s;
    memcpy((char *)&s, buf, sizeof(s));
    d = data_t(s);
    memcpy((char *)d.data(), buf + sizeof(s), s);
    return s + sizeof(s);
  }
  static uint32_t length(const data_t &d) { return d.size() + sizeof(size_t); }
};
} // namespace net
} // namespace ff

namespace std {
template <typename ByteType, ::ypc::utc::byte_encode Format>
struct hash<::ypc::utc::bytes<ByteType, Format>> {
  typedef ::ypc::utc::bytes<ByteType, Format> argument_type;
  typedef std::size_t result_type;

  result_type operator()(argument_type const &s) const noexcept {
    return std::hash<std::string>{}(
        std::string((const char *)s.data(), s.size()));
  }
};
} // namespace std
