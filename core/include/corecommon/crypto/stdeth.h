#pragma once
#include "corecommon/crypto/stdeth/eth_hash.h"
#include "corecommon/crypto/stdeth/rijndael128GCM.h"
#include "corecommon/crypto/stdeth/secp256k1.h"
#include "corecommon/crypto/stdeth/secp256k1_ecdh_sgx128.h"

#include "corecommon/crypto/crypto_pack.h"

namespace ypc {
namespace crypto {
typedef crypto_pack<secp256k1, eth_hash, rijndael128GCM, secp256k1_ecdh_sgx128>
    eth_sgx_crypto;
}
} // namespace ypc
