#pragma once
#include "common/crypto_prefix.h"
#include "stbox/stx_status.h"

namespace ypc {

template <typename Crypto> struct allowance {
  typedef Crypto ecc;

  template <typename BytesType>
  static uint32_t check(const BytesType &private_key, const BytesType &msg,
                        const BytesType allowance) {
    BytesType decrypted_allowance;
    uint32_t ret = ecc::decrypt_message_with_prefix(
        private_key, allowance, decrypted_allowance,
        ypc::utc::crypto_prefix_arbitrary);
    if (ret) {
      return ret;
    }

    BytesType pkey;
    ret = ecc::generate_pkey_from_skey(private_key, pkey);
    if (ret) {
      return ret;
    }
    ret = ecc::verify_signature(msg, decrypted_allowance, pkey);
    return ret;
  }

  template <typename BytesType>
  static uint32_t generate(const BytesType &private_key, const BytesType &msg,
                           BytesType &allowance) {
    BytesType sig;
    uint32_t ret = ecc::sign_message(private_key, msg, sig);
    if (ret) {
      return ret;
    }

    BytesType pkey;
    ret = ecc::generate_pkey_from_skey(private_key, pkey);
    if (ret) {
      return ret;
    }

    ret = ecc::encrypt_message_with_prefix(
        pkey, sig, ypc::utc::crypto_prefix_arbitrary, allowance);
    return ret;
  }
};

} // namespace ypc
