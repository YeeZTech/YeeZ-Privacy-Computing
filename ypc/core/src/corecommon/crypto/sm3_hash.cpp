#include "ypc/corecommon/crypto/gmssl/sm3_hash.h"
#include "ypc/common/byte.h"
#include "ypc/stbox/gmssl/sha2.h"
#include "ypc/stbox/gmssl/sm3.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace crypto {

uint32_t sm3_hash::hash_256(const uint8_t *msg, uint32_t msg_size,
                            uint8_t *hash) {
  SM3_CTX ctx;
  sm3_init(&ctx);
  sm3_update(&ctx, msg, msg_size);
  sm3_finish(&ctx, hash);
  return stbox::stx_status::success;
}

uint32_t sm3_hash::msg_hash(const uint8_t *raw_msg, uint32_t msg_size,
                            uint8_t *hash, uint32_t hash_size) {
  return hash_256(raw_msg, msg_size, hash);
}
} // namespace crypto
} // namespace ypc
