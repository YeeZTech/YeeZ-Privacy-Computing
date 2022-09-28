#pragma once
#include "ypc/common/crypto_prefix.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {

template <typename Crypto> struct allowance {
  typedef Crypto ecc;

  template <typename BytesType>
  static uint32_t check(const BytesType &private_key, const BytesType &msg,
                        const BytesType &allowance) {
    //check(private_key, to_check_data, allow);
    BytesType pkey;
    auto ret = ecc::generate_pkey_from_skey(private_key, pkey);
    if (ret) {
      LOG(INFO) << "return without verify signature " << ret;
      return ret;
    }
    LOG(INFO) << "ret after verify";
    LOG(INFO) << msg;
    LOG(INFO) << allowance;
    LOG(INFO) << pkey;
    ret = ecc::verify_signature(msg, allowance, pkey);
    LOG(INFO) << "the res after vefify" << ret;
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
