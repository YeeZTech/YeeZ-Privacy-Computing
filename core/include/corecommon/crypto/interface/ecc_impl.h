#pragma once
#include "common/byte.h"

namespace ypc {
namespace crypto {
namespace internal {
template <typename EC> class ecc_impl {
  typedef EC ecc_t;

public:
  static uint32_t get_private_key_size() {
    return ecc_t::get_private_key_size();
  }
  static uint32_t get_public_key_size() { return ecc_t::get_public_key_size(); }
  static uint32_t gen_private_key(uint32_t skey_size, uint8_t *skey) {
    return ecc_t::gen_privae_key(skey_size, skey);
  }

  template <typename BytesType>
  static uint32_t gen_private_key(BytesType &skey) {
    skey = BytesType(get_private_key_size());
    return ecc_t::gen_private_key(skey.size(), skey.data());
  }

  static uint32_t generate_pkey_from_skey(const uint8_t *skey,
                                          uint32_t skey_size, uint8_t *pkey,
                                          uint32_t pkey_size) {
    return ecc_t::generate_pkey_from_skey(skey, skey_size, pkey, pkey_size);
  }

  template <typename BytesType>
  static uint32_t generate_pkey_from_skey(const BytesType &skey,
                                          BytesType &pkey) {
    pkey = BytesType(get_public_key_size());
    return ecc_t::generate_pkey_from_skey(skey.data(), skey.size(), pkey.data(),
                                          pkey.size());
  }
};
} // namespace internal
} // namespace crypto
} // namespace ypc
