#pragma once
#include "stbox/ebyte.h"
extern "C" {
#include "stbox/keccak/keccak.h"
}
#define KECCAK256_HASH_SIZE 32

namespace stbox {
namespace eth {

template <typename BytesType> bytes keccak256_hash(const BytesType &msg) {
  sha3_context c;
  uint8_t *hash;
  sha3_Init256(&c);
  sha3_SetFlags(&c, SHA3_FLAGS_KECCAK);
  sha3_Update(&c, msg.data(), msg.size());
  hash = (uint8_t *)sha3_Finalize(&c);
  return bytes(hash, KECCAK256_HASH_SIZE);
}

bytes eth_msg(const bytes &message);
bytes msg_hash(const uint8_t *raw_msg, uint32_t msg_size);
} // namespace eth
} // namespace stbox
