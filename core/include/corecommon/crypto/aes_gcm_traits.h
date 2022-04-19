#pragma once

namespace ypc {
namespace crypto {

template <typename T> struct aes_gcm_traits {
  constexpr static bool value = false;
};
} // namespace crypto
} // namespace ypc
