#include "corecommon/crypto/stdeth/secp256k1.h"
#include "corecommon/crypto/stdeth/secp256k1.ipp"
//#include <sgx_tcrypto.h>
#include <sgx_trts.h>

namespace ypc {
namespace crypto {
uint32_t secp256k1::gen_private_key(uint32_t skey_size, uint8_t *skey) {
  secp256k1_context *ctx = init_secp256k1_context();

  if (!ctx) {
    LOG(ERROR) << "Context or Secret key or Public key is null";
    return stbox::stx_status::ecc_invalid_ctx_or_skey;
  }
  sgx_status_t se_ret;
  do {
    se_ret = sgx_read_rand(skey, skey_size);
    if (se_ret != SGX_SUCCESS) {
      LOG(ERROR) << "call sgx_read_rand failed";
      return se_ret;
    }
  } while (!secp256k1_ec_seckey_verify(ctx, skey));
  return se_ret;
}
} // namespace crypto
} // namespace ypc
