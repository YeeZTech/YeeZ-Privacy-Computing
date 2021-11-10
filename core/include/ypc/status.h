#pragma once
#include <string>

namespace ypc {
enum ypc_status : uint32_t {
#define YPC_STATUS(a, b) a = (b == 0 ? b : 0x20000 | b),
#include "ypc/status.def"
#undef YPC_STATUS
};
const char *status_string(uint32_t status);
} // namespace ypc
