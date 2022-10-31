#pragma once
#include "ypc/corecommon/crypto/crypto_pack.h"
#include "ypc/corecommon/crypto/stdeth/eth_hash.h"
#include "ypc/corecommon/crypto/stdeth/rijndael128GCM.h"
#include "ypc/corecommon/crypto/stdeth/secp256k1.h"
#include "ypc/corecommon/crypto/stdeth/secp256k1_ecdh_sgx128.h"

namespace ypc {
namespace crypto {
typedef crypto_pack<secp256k1, eth_hash, rijndael128GCM, secp256k1_ecdh_sgx128>
    eth_sgx_crypto;
}
} // namespace ypc
