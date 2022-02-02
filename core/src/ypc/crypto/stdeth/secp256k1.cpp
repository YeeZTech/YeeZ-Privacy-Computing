#include "corecommon/crypto/stdeth/secp256k1.h"
#include "corecommon/crypto/stdeth/secp256k1.ipp"
#include <openssl/rand.h>

namespace ypc {
namespace crypto {
uint32_t secp256k1::gen_private_key(uint32_t skey_size, uint8_t *skey) {
  secp256k1_context *ctx = init_secp256k1_context();

  if (!ctx) {
    LOG(ERROR) << "Context or Secret key or Public key is null";
    return stbox::stx_status::ecc_invalid_ctx_or_skey;
  }
  int counter = 0;
  do {
    auto rc = RAND_bytes(skey, skey_size);
    if (rc != 1) {
      throw std::runtime_error("RAND_bytes key failed");
    }
    counter++;
    if (counter > 0xFFFF) {
      throw std::runtime_error(
          "secp256k1::gen_private_key too many times, you may retry");
    }
  } while (!secp256k1_ec_seckey_verify(ctx, skey));
  return stbox::stx_status::success;
}
} // namespace crypto
} // namespace ypc
