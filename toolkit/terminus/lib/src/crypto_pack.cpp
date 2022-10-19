#include "toolkit/terminus/lib/include/ypc/terminus/crypto_pack.h"
#include "ypc/corecommon/crypto/crypto_pack.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/crypto/gmssl.h"

namespace ypc {
namespace terminus {

std::unique_ptr<crypto_pack> intel_sgx_and_eth_compatible() {
  return std::unique_ptr<crypto_pack>(
      new crypto_pack_t<::ypc::crypto::eth_sgx_crypto>());
}

std::unique_ptr<crypto_pack> sm_compatible() {
  return std::unique_ptr<crypto_pack>(
      new crypto_pack_t<::ypc::crypto::gmssl_sgx_crypto>());
}

} // namespace terminus
} // namespace ypc