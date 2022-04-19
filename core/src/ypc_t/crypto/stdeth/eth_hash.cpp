#include "corecommon/crypto/stdeth/eth_hash.h"
#include "common/byte.h"
#include "stbox/stx_status.h"
extern "C" {
#include "stbox/keccak/keccak.h"
}
namespace ypc {
namespace crypto {
#define KECCAK256_HASH_SIZE 32

typedef ::ypc::utc::bytes<uint8_t, ::ypc::utc::byte_encode::raw_bytes> bytes;
static bytes keccak256_hash(const bytes &msg) {
  sha3_context c;
  uint8_t *hash;
  sha3_Init256(&c);
  sha3_SetFlags(&c, SHA3_FLAGS_KECCAK);
  sha3_Update(&c, msg.data(), msg.size());
  hash = (uint8_t *)sha3_Finalize(&c);
  return bytes(hash, KECCAK256_HASH_SIZE);
}
static bytes eth_msg(const bytes &message) {
  // Ethereum message prefix "\x19Ethereum Signed Message:\n32"
  bytes msg({0x19});
  bytes tmp("Ethereum Signed Message:\n32");
  msg = msg + tmp;
  auto raw_hash = keccak256_hash(message);
  msg = msg + raw_hash;
  return msg;
}

uint32_t eth_hash::sha3_256(const uint8_t *msg, uint32_t msg_size,
                            uint8_t *hash) {
  sha3_context c;
  sha3_Init256(&c);
  sha3_SetFlags(&c, SHA3_FLAGS_KECCAK);
  sha3_Update(&c, msg, msg_size);
  uint8_t *_hash = (uint8_t *)sha3_Finalize(&c);
  memcpy(hash, _hash, 32);
  return stbox::stx_status::success;
}

uint32_t eth_hash::get_msg_hash_size() { return 32; }

uint32_t eth_hash::msg_hash(const uint8_t *raw_msg, uint32_t msg_size,
                            uint8_t *hash, uint32_t hash_size) {
  auto msg = eth_msg(bytes(raw_msg, msg_size));
  bytes ret = keccak256_hash(msg);
  memcpy(hash, ret.data(), hash_size);
  return stbox::stx_status::success;
}
} // namespace crypto
} // namespace ypc

