#pragma once
#include "stbox/ebyte.h"

namespace stbox {
namespace eth {
hex_bytes checksum_addr(const hex_bytes &addr);
hex_bytes gen_addr_from_pkey(const bytes &pkey);
} // namespace eth
} // namespace stbox
