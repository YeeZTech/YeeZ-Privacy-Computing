#pragma once
#include "common/byte.h"

namespace ypc {
namespace crypto {
namespace internal {

template <typename Hash> struct hash_impl {
  typedef Hash hash_t;

public:
  template <typename BytesType>
  static uint32_t sha3_256(const BytesType &msg, BytesType &sig) {
    sig = BytesType(32);
    return hash_t::sha3_256(msg.data(), msg.size(), sig.data());
  }
  static uint32_t get_msg_hash_size() { return hash_t::get_msg_hash_size(); }
  static uint32_t msg_hash(const uint8_t *raw_msg, uint32_t msg_size,
                           uint8_t *hash, uint32_t hash_size) {
    return hash_t::msg_hash(raw_msg, msg_size, hash, hash_size);
  }
  template <typename BytesType>
  static uint32_t msg_hash(const BytesType &raw_msg, BytesType &hash) {
    hash = BytesType(hash_t::get_msg_hash_size());
    return hash_t::msg_hash(raw_msg.data(), raw_msg.size(), hash.data(),
                            hash.size());
  }
};

} // namespace internal
} // namespace crypto
} // namespace ypc
