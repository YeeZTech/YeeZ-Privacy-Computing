#pragma once
#include "common/byte.h"

namespace ypc {
namespace crypto {
namespace internal {
template <typename ECDH> struct ecdh_impl {
  typedef ECDH ecdh_t;

public:
  static uint32_t get_ecdh_shared_key_size() {
    return ecdh_t::get_ecdh_shared_key_size();
  }

  static uint32_t ecdh_shared_key(const uint8_t *skey, uint32_t skey_size,
                                  const uint8_t *public_key, uint32_t pkey_size,
                                  uint8_t *shared_key,
                                  uint32_t shared_key_size) {
    return ecdh_t::ecdh_shared_key(skey, skey_size, public_key, pkey_size,
                                   shared_key, shared_key_size);
  }
  template <typename BytesType>
  static uint32_t ecdh_shared_key(const BytesType &skey,
                                  const BytesType &public_key,
                                  BytesType &shared_key) {
    shared_key = BytesType(ecdh_t::get_ecdh_shared_key_size());
    return ecdh_t::ecdh_shared_key(skey.data(), skey.size(), public_key.data(),
                                   public_key.size(), shared_key.data(),
                                   shared_key.size());
  }
};
} // namespace internal
} // namespace crypto
} // namespace ypc
