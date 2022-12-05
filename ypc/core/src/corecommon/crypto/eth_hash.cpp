#include "ypc/corecommon/crypto/stdeth/eth_hash.h"
#include "ypc/common/byte.h"
#include "ypc/stbox/stx_status.h"
extern "C" {
// NOLINTNEXTLINE
#include "ypc/stbox/keccak/keccak.h"
}
namespace ypc {
namespace crypto {
#define KECCAK256_HASH_SIZE 32

typedef ::ypc::utc::bytes<uint8_t, ::ypc::utc::byte_encode::raw_bytes> bytes;

uint32_t eth_hash::hash_256(const uint8_t *msg, uint32_t msg_size,
                            uint8_t *hash) {
  sha3_context c;
  sha3_Init256(&c);
  sha3_SetFlags(&c, SHA3_FLAGS_KECCAK);
  sha3_Update(&c, msg, msg_size);
  auto *_hash = (uint8_t *)sha3_Finalize(&c);
  memcpy(hash, _hash, 32);
  return stbox::stx_status::success;
}

uint32_t eth_hash::get_msg_hash_size() { return 32; }

uint32_t eth_hash::msg_hash(const uint8_t *raw_msg, uint32_t msg_size,
                            uint8_t *hash, uint32_t hash_size) {
  // prefix string "\x19Ethereum Signed Message:\n32"
  const static char prefix[] = {0x19, 0x45, 0x74, 0x68, 0x65, 0x72, 0x65,
                                0x75, 0x6d, 0x20, 0x53, 0x69, 0x67, 0x6e,
                                0x65, 0x64, 0x20, 0x4d, 0x65, 0x73, 0x73,
                                0x61, 0x67, 0x65, 0x3a, 0x0a, 0x33, 0x32};
  const uint32_t l = sizeof(prefix) / sizeof(prefix[0]);
  bytes msg(l + 32);
  memcpy(msg.data(), prefix, l);
  auto r = hash_256(raw_msg, msg_size, msg.data() + l);
  if (r != stbox::stx_status::success) {
    return r;
  }

  return hash_256(msg.data(), msg.size(), hash);
}
} // namespace crypto
} // namespace ypc

