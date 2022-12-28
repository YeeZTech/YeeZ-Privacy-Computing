#pragma once
#include <cstddef>
#include <cstdint>

namespace ypc {
// using fpos = std::array<uint8_t, 16>;

namespace ios_base {
typedef uint8_t seekdir;
static constexpr seekdir beg = 0;
static constexpr seekdir end = 1;
static constexpr seekdir cur = 2;
} // namespace ios_base

} // namespace ypc
