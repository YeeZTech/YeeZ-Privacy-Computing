#pragma once
#include "corecommon/nt_cols.h"
#include "corecommon/package.h"
#include "stbox/tsgx/channel/dh_session.h"

namespace ypc {
bool is_certified_signer(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity,
    std::shared_ptr<nt<stbox::bytes>::access_list_package_t> policy =
        std::shared_ptr<nt<stbox::bytes>::access_list_package_t>());
}
