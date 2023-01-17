#include "ypc/corecommon/crypto/stdeth.h"

using ecc = ypc::crypto::eth_sgx_crypto;

#ifdef DEBUG
#define KEY_PATH ".yeez.stdeth_key.debug"
#else
#define KEY_PATH ".yeez.stdeth_key"
#endif

#include "ypc/keymgr/default/ekeymgr.ipp"
