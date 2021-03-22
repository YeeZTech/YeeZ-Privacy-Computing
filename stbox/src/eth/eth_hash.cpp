#include "stbox/eth/eth_hash.h"
extern "C" {
#include "stbox/keccak/keccak.h"
}

#define KECCAK256_HASH_SIZE 32

namespace stbox {
namespace eth {

bytes keccak256_hash(const bytes &msg) {
  sha3_context c;
  uint8_t *hash;
  sha3_Init256(&c);
  sha3_SetFlags(&c, SHA3_FLAGS_KECCAK);
  sha3_Update(&c, msg.value(), msg.size());
  hash = (uint8_t *)sha3_Finalize(&c);
  return bytes(hash, KECCAK256_HASH_SIZE);
}

bytes eth_msg(const bytes &message) {
  // Ethereum message prefix "\x19Ethereum Signed Message:\n32"
  bytes msg({0x19});
  std::string tmp = "Ethereum Signed Message:\n32";
  msg = msg + string_to_byte(tmp);
  auto raw_hash = keccak256_hash(message);
  msg = msg + raw_hash;
  return msg;
}

bytes msg_hash(const uint8_t *raw_msg, uint32_t msg_size) {
  auto msg = eth_msg(bytes(raw_msg, msg_size));
  return keccak256_hash(msg);
}

} // namespace eth
} // namespace stbox
