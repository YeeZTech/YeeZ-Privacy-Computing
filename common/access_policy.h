#pragma once
#include <cstdint>

namespace ypc {
namespace utc {
constexpr static uint8_t access_policy_whitelist = 1;
constexpr static uint8_t access_policy_blacklist = 2;
constexpr static uint8_t access_policy_signer = 3;
constexpr static uint8_t access_policy_enclave = 4;

} // namespace utc
} // namespace ypc
