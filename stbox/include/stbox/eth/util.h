
#pragma once
#include "stbox/ebyte.h"

namespace stbox {
namespace eth {

void checksum_addr(std::string &addr);
std::string gen_addr_from_pkey(const bytes &pkey);
} // namespace eth
} // namespace stbox
