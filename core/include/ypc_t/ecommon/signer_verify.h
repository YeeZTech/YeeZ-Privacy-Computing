#pragma once
#include "stbox/tsgx/channel/dh_session.h"

namespace ypc {
bool is_certified_signer(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity);
}
