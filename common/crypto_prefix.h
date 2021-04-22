#pragma once
#include <cstdint>

namespace ypc {

typedef uint32_t crypto_prefix_t;

constexpr static uint32_t crypto_prefix_length = sizeof(uint32_t);

constexpr static crypto_prefix_t crypto_prefix_forward = 0x1;
constexpr static crypto_prefix_t crypto_prefix_arbitrary = 0x2;
constexpr static crypto_prefix_t crypto_prefix_backup = 0x4;

} // namespace ypc
