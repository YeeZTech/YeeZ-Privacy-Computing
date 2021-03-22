
#pragma once
#include "stbox/ebyte.h"

namespace stbox {
namespace eth {

bytes keccak256_hash(const bytes &msg);
bytes eth_msg(const bytes &message);
bytes msg_hash(const uint8_t *raw_msg, uint32_t msg_size);
} // namespace eth
} // namespace stbox
