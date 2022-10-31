#pragma once
#include "ypc/corecommon/crypto/gmssl/sm2_ecc.h"
#include "ypc/corecommon/crypto/gmssl/sm3_hash.h"
#include "ypc/corecommon/crypto/gmssl/sm4_aes.h"
#include "ypc/corecommon/crypto/crypto_pack.h"

namespace ypc {
namespace crypto {
typedef crypto_pack<sm2_ecc, sm3_hash, sm4_aes, sm2_ecc>
    gmssl_sgx_crypto;
}
} // namespace ypc
