#pragma once
#include <cstdint>

namespace ypc {
namespace crypto {

class sm3_hash {
public:
  static uint32_t sha3_256(const uint8_t *msg, uint32_t msg_size, uint8_t *hash);

  inline static uint32_t get_msg_hash_size() { return 32; }

  static uint32_t msg_hash(const uint8_t *raw_msg, uint32_t msg_size,
                           uint8_t *hash, uint32_t hash_size);
};

} // namespace crypto
} // namespace ypc
