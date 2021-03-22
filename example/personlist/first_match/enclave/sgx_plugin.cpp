#include "sgx_plugin.h"
#include <cstdint>
#include <stdexcept>

uint64_t ypc_plugin_version() { return 1; }

namespace ypc {
user_item_t ecall_parse_item_data(const char *data, size_t len) {
  user_item_t ret;
  ff::net::marshaler m(data, len, ff::net::marshaler::deseralizer);
  ret.arch(m);
  return ret;
}
} // namespace ypc
