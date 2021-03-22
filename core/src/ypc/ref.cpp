#include "ypc/ref.h"
#include "ypc/byte.h"

namespace ypc {
std::string to_hex(const bref &s) {
  return ::ypc::internal::convert_byte_to_hex(s.data(), s.len());
}
} // namespace ypc
