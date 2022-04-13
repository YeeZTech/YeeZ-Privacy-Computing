#include "ypc/status.h"
#include "stbox/stx_status.h"

namespace ypc {
const char *status_string(uint32_t status) {
  if (!status) {
    return "success";
  }
  if (status & 0x20000) {
#define YPC_STATUS(a, b)                                                       \
  case 0x20000 | b:                                                            \
    return #a;

    switch (status) {
#include "ypc/status.def"
    default:
      return "unknown ypc status";
    }

#undef YPC_STATUS
  } else {
    // defined in stbox status
    return "unknown ypc status";
    // return ::stbox::status_string(status);
  }
}
} // namespace ypc
