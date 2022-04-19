#pragma once
#include "common/crypto_prefix.h"
#include "stbox/stx_status.h"

namespace ypc {

template <typename Crypto> struct allowance {
  typedef Crypto ecc;

  template <typename BytesType>
  static uint32_t check(const BytesType &private_key, const BytesType &msg,
                        const BytesType &allowance) {
    BytesType pkey;
    auto ret = ecc::generate_pkey_from_skey(private_key, pkey);
    if (ret) {
      return ret;
    }
    ret = ecc::verify_signature(msg, allowance, pkey);
    return ret;
  }

  template <typename BytesType>
  static uint32_t generate(const BytesType &private_key, const BytesType &msg,
                           BytesType &allowance) {
    uint32_t ret = ecc::sign_message(private_key, msg, allowance);
    return ret;
  }
};

} // namespace ypc
