#pragma once
#include <type_traits>
namespace ypc {
namespace ios_base {
enum openmode {
  app = 0x1,
  ate = 0x2,
  binary = 0x4,
  in = 0x8,
  out = 0x10,
  trunc = 0x20
};
inline openmode operator|(openmode a, openmode b) {
  typedef std::underlying_type<openmode>::type UL;
  return openmode(static_cast<UL>(a) | static_cast<UL>(b));
}
inline openmode operator&(openmode a, openmode b) {
  typedef std::underlying_type<openmode>::type UL;
  return openmode(static_cast<UL>(a) & static_cast<UL>(b));
}
}
} // namespace ypc
