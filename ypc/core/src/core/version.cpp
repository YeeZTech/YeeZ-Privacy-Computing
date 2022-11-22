#include "ypc/core/version.h"
#include "ypc/version.h"

namespace ypc {
std::string get_ypc_version() { return std::to_string(YPC_RUNTIME_VERSION); }
}
