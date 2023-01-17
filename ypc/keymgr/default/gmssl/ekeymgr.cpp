#include "ypc/corecommon/crypto/gmssl.h"

using ecc = ypc::crypto::gmssl_sgx_crypto;

#ifdef DEBUG
#define KEY_PATH ".yeez.gmssl_key.debug"
#else
#define KEY_PATH ".yeez.gmssl_key"
#endif

#include "ypc/keymgr/default/ekeymgr.ipp"
