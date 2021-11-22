#include "ypc/terminus/crypto_pack.h"

namespace ypc {
namespace terminus {
bytes crypto_pack::sign_message(const bytes &message,
                                const bytes &private_key) {
  return sign_hash(chain_hash(message), private_key);
}

bool crypto_pack::verify_message_signature(const bytes &sig,
                                           const bytes &message,
                                           const bytes &pubkey) {
  auto hash = chain_hash(message);
  return verify_hash_signature(sig, hash, pubkey);
}
} // namespace terminus
} // namespace ypc
