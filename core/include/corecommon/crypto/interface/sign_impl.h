#pragma once
#include "common/byte.h"

namespace ypc {
namespace crypto {
namespace internal {
template <typename Hash, typename EC> struct sign_impl {
  typedef Hash hash_t;
  typedef EC ecc_t;
  typedef ::ypc::utc::bytes<uint8_t, ::ypc::utc::byte_encode::raw_bytes> bytes;

public:
  static uint32_t get_signature_size() { return ecc_t::get_signature_size(); }

  static uint32_t sign_message(const uint8_t *skey, uint32_t skey_size,
                               const uint8_t *data, uint32_t data_size,
                               uint8_t *sig, uint32_t sig_size) {
    bytes hash(hash_t::get_msg_hash_size());
    uint32_t ret = hash_t::msg_hash(data, data_size, hash.data(), hash.size());
    if (ret) {
      return ret;
    }
    ret = ecc_t::sign_message(skey, skey_size, hash.data(), hash.size(), sig,
                              sig_size);
    return ret;
  }

  static uint32_t verify_signature(const uint8_t *data, uint32_t data_size,
                                   const uint8_t *sig, uint32_t sig_size,
                                   const uint8_t *public_key,
                                   uint32_t pkey_size) {
    uint32_t hash_size = hash_t::get_msg_hash_size();
    bytes hash(hash_size);
    uint32_t ret = hash_t::msg_hash(data, data_size, hash.data(), hash_size);
    if (ret) {
      return ret;
    }
    ret = ecc_t::verify_signature(hash.data(), hash_size, sig, sig_size,
                                  public_key, pkey_size);
    return ret;
  }

  template <typename BytesType>
  static uint32_t sign_message(const BytesType &skey, const BytesType &data,
                               BytesType &sig) {
    sig = BytesType(ecc_t::get_signature_size());
    return sign_message(skey.data(), skey.size(), data.data(), data.size(),
                        sig.data(), sig.size());
  }

  template <typename BytesType>
  static uint32_t verify_signature(const BytesType &data, const BytesType &sig,
                                   const BytesType &public_key) {
    return verify_signature(data.data(), data.size(), sig.data(), sig.size(),
                            public_key.data(), public_key.size());
  }
};
} // namespace internal
} // namespace crypto
} // namespace ypc
