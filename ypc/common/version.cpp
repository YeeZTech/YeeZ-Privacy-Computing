#include "ypc/common/version.h"
#ifndef YPC_SGX
#include <sstream>
#endif
#include <exception>

#ifndef YPC_SGX
std::ostream &operator<<(std::ostream &stream, const ypc::version &v) {
  stream << v.major_version() << '.' << v.minor_version() << '.'
         << v.patch_version();
  return stream;
}

std::istream &operator>>(std::istream &stream, ypc::version &v) {
  uint32_t major;
  uint16_t minor;
  uint16_t patch;
  char c;
  stream >> major >> c;
  if (c != '.') {
    throw std::runtime_error("invalid sep in version, expect '.'");
  }
  stream >> minor >> c;
  if (c != '.') {
    throw std::runtime_error("invalid sep in version, expect '.'");
  }
  stream >> patch;
  v = ypc::version(major, minor, patch);
  return stream;
}

#endif

namespace std {
std::string to_string(const ypc::version &v) {
  return std::to_string(v.major_version()) + "." +
         std::to_string(v.minor_version()) + "." +
         std::to_string(v.patch_version());
}
} // namespace std

