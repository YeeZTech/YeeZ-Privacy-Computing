#pragma once
#include <sgx_error.h>
#include <string>

namespace stbox {
enum class stx_status : uint16_t {
#define ATT_STATUS(a, b) a = b,

#include "stbox/stx_status.def"

#undef ATT_STATUS
};

const char *sgx_status_string(sgx_status_t status);
} // namespace stbox

namespace std {
std::string to_string(stbox::stx_status status);
}


