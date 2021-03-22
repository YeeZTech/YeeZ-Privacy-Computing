#pragma once
#include <sgx_edger8r.h>
#include <string>

namespace std {
std::string to_string(sgx_status_t ret);
}
