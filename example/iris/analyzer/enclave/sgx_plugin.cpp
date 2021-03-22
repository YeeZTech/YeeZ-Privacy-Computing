#include "sgx_plugin.h"
#include "iris_parser.h"
#include <cstdint>
#include <stdexcept>

uint64_t ypc_plugin_version() { return 1; }

namespace ypc {
user_item_t ecall_parse_item_data(const char *data, size_t len) {
  user_item_t ret;
  int r = parse_item_data(data, len, &ret);
  if (r) {
    throw std::runtime_error("error");
  }
  return ret;
}
} // namespace ypc
