#include "ypc/to_hex.h"
#include "ypc/byte.h"

namespace ypc {
std::string to_hex(const std::string &s) {
  return ::ypc::internal::convert_byte_to_hex((const byte_t *)s.c_str(),
                                              s.size());
}
} // namespace ypc
