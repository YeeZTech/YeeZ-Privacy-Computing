#include "ecc_t.h"
#include "ypc/common/crypto_prefix.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/stbox/tsgx/crypto/seal.h"
#include "ypc/stbox/tsgx/crypto/seal_sgx.h"
#include "ypc/stbox/tsgx/log.h"

using ecc = ypc::crypto::gmssl_sgx_crypto;
using sealer = stbox::crypto::raw_device_sealer<stbox::crypto::intel_sgx>;

#include "ecc_impl.ipp"
