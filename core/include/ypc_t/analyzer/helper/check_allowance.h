#pragma once
#include "common/crypto_prefix.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"

namespace ypc {
namespace internal {
template <typename Crypto>
uint32_t check_allowance(const stbox::bytes &private_key,
                         const stbox::bytes &msg,
                         const stbox::bytes allowance) {
  typedef Crypto ecc;
  stbox::bytes decrypted_allowance;
  uint32_t ret = ecc::decrypt_message_with_prefix(
      private_key, allowance, decrypted_allowance,
      ypc::utc::crypto_prefix_arbitrary);
  if (ret) {
    LOG(ERROR) << "decrypt_message_with_prefix failed: "
               << stbox::status_string(ret);
    return ret;
  }

  stbox::bytes pkey;
  ret = ecc::generate_pkey_from_skey(private_key, pkey);
  if (ret) {
    LOG(ERROR) << "generate_pkey_from_skey failed: "
               << stbox::status_string(ret);
    return ret;
  }
  ret = ecc::verify_signature(msg, decrypted_allowance, pkey);
  if (ret) {
    LOG(ERROR) << "verify_signature failed: " << stbox::status_string(ret);
    return ret;
  }

  return stbox::stx_status::success;
}

} // namespace internal
} // namespace ypc
