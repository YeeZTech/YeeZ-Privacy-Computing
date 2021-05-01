#include "stbox/eth/eth_hash.h"


namespace stbox {
namespace eth {

bytes eth_msg(const bytes &message) {
  // Ethereum message prefix "\x19Ethereum Signed Message:\n32"
  bytes msg({0x19});
  bytes tmp("Ethereum Signed Message:\n32");
  msg = msg + tmp;
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
