#pragma once
#include "sgx_ecp_types.h"
#include "sgx_tcrypto.h"

namespace stbox {
namespace crypto {
sgx_status_t derive_key(const sgx_ec256_dh_shared_t *shared_key,
                        const char *label, uint32_t label_length,
                        sgx_ec_key_128bit_t *derived_key);
}
}


