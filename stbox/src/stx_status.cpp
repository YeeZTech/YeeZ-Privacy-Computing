#include "stbox/stx_status.h"
#include <stdexcept>

namespace stbox {
const char *status_string(uint32_t status) {
  if (status & 0x10000) {
#define ATT_STATUS(a, b)                                                       \
  case b | 0x10000:                                                            \
    return #a;

    switch (status) {
#include "stbox/stx_status.def"
    default:
      return "unknown stx status";
    }

#undef ATT_STATUS
  } else {
    switch (status) {
#define SGX_STATUS(n)                                                          \
  case n:                                                                      \
    return #n;
#include <stbox/sgx_status.def>
#undef SGX_STATUS
  default:
    return "unknown sgx status";
  }
  return "unknown sgx status";
  }
}
} // namespace stbox

