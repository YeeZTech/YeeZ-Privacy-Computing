#pragma once
#include <cstdint>

namespace ypc {
namespace utc {
typedef uint32_t crypto_prefix_t;

constexpr static uint32_t crypto_prefix_length = sizeof(uint32_t);

constexpr static crypto_prefix_t crypto_prefix_forward = 0x1;
constexpr static crypto_prefix_t crypto_prefix_arbitrary = 0x2;
constexpr static crypto_prefix_t crypto_prefix_backup = 0x4;
constexpr static crypto_prefix_t crypto_prefix_host_data = 0x6;
constexpr static crypto_prefix_t crypto_prefix_host_data_private_key = 0x8;

} // namespace utc
} // namespace ypc
