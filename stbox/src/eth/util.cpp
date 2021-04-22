#include "stbox/eth/util.h"
#include "common/util.h"
#include "stbox/eth/eth_hash.h"

namespace stbox {

namespace eth {

void checksum_addr(std::string &addr) {
  // ATTENTION! NOT bytes::from_hex(addr), BUT string_to_byte(addr)
  auto hash = keccak256_hash(string_to_byte(addr)).to_hex();
  for (size_t i = 0; i < addr.size(); i++) {
    auto &ch = addr[i];
    if (hash[i] >= '8') {
      ch = toupper(ch);
    }
  }
  addr = "0x" + addr;
}


std::string gen_addr_from_pkey(const bytes &pkey) {
  bytes be_pkey(pkey);
  ypc::change_endian(be_pkey);
  auto hash = keccak256_hash(be_pkey).to_hex();
  auto addr = hash.substr(hash.size() - 40);
  checksum_addr(addr);
  return addr;
}

} // namespace eth
} // namespace stbox
