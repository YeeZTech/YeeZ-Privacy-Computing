#include "ecc_t.h"
#include "ypc/common/crypto_prefix.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/stbox/tsgx/crypto/seal.h"
#include "ypc/stbox/tsgx/crypto/seal_sgx.h"
#include "ypc/stbox/tsgx/log.h"

using ecc = ypc::crypto::eth_sgx_crypto;
using raw_ecc = ecc;
using sealer = stbox::crypto::raw_device_sealer<stbox::crypto::intel_sgx>;

#include "ecc_impl.ipp"