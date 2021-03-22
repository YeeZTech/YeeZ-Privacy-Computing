#include "stbox/stx_status.h"
#include <stdexcept>

namespace stbox {
const char *sgx_status_string(sgx_status_t status) {
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

} // namespace stbox
namespace std {
std::string to_string(::stbox::stx_status status) {
#define ATT_STATUS(a, b)                                                       \
  case ::stbox::stx_status::a:                                                 \
    return #a;

  switch (status) {
#include "stbox/stx_status.def"
  default:
    throw std::runtime_error("invalid stx_status");
  }
#undef ATT_STATUS
}

} // namespace std
