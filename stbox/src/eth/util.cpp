#include "stbox/eth/util.h"
#include "common/endian.h"
#include "stbox/eth/eth_hash.h"

namespace stbox {

namespace eth {

hex_bytes checksum_addr(const hex_bytes &addr) {
  auto hash = keccak256_hash(addr).as<hex_bytes>();
  hex_bytes ret(addr);
  for (size_t i = 0; i < addr.size(); i++) {
    auto &ch = ret[i];
    if (hash[i] >= '8') {
      ch = toupper(ch);
    }
  }
  return ret;
}

hex_bytes gen_addr_from_pkey(const bytes &pkey) {
  bytes be_pkey(pkey);
  ypc::utc::change_pubkey_endian(be_pkey);
  auto hash = keccak256_hash(be_pkey).as<hex_bytes>();
  hex_bytes sub(hash.data() + 40, hash.size() - 40);
  return checksum_addr(sub);
}

} // namespace eth
} // namespace stbox
