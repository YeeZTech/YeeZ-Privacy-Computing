#include "common.h"
#include "ekeymgr_t.h" /* print_string */
#include "ypc/common/crypto_prefix.h"
#include "ypc/core_t/ecommon/signer_verify.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/eth/eth_hash.h"
#include "ypc/stbox/scope_guard.h"
#include "ypc/stbox/stx_common.h"
#include "ypc/stbox/tsgx/channel/dh_session_responder.h"
#include "ypc/stbox/tsgx/crypto/seal.h"
#include "ypc/stbox/tsgx/crypto/seal_sgx.h"
#include "ypc/stbox/tsgx/log.h"

#include <sgx_report.h>
#include <sgx_utils.h>

#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

#define SECP256K1_PRIVATE_KEY_SIZE 32
#define INITIALIZATION_VECTOR_SIZE 12
#define SGX_AES_GCM_128BIT_TAG_T_SIZE sizeof(sgx_aes_gcm_128bit_tag_t)

extern "C" {
#include "ypc/stbox/keccak/keccak.h"
}

using stx_status = stbox::stx_status;
using scope_guard = stbox::scope_guard;
using intel_sgx = stbox::crypto::intel_sgx;
using ecc = ypc::crypto::eth_sgx_crypto;
// using raw_ecc = ecc;
// using namespace stbox;
// using namespace stbox::crypto;

std::shared_ptr<stbox::dh_session_responder> dh_resp_session(nullptr);
std::shared_ptr<ypc::nt<stbox::bytes>::access_list_package_t>
    access_control_policy;

#define KEY_PATH ".yeez.stdeth_key"

#include "../ekeymgr.ipp"

