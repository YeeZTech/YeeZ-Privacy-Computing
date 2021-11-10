#pragma once
#include <sgx_error.h>
#include <string>

namespace stbox {
enum stx_status : uint32_t {
// 0x10000 makes sure our error code won't overlap with sgx_status_t
#define ATT_STATUS(a, b) a = (b == 0 ? b : 0x10000 | b),

#include "stbox/stx_status.def"

#undef ATT_STATUS
};

const char *status_string(uint32_t status);
} // namespace stbox

