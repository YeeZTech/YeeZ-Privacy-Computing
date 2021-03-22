#include "stbox/stx_common.h"
#include <cstdint>
#include <stdlib.h>

namespace stbox {
void print_hex(uint8_t *data, size_t data_len) {
  for (size_t i = 0; i < data_len; i++) {
    printf("%02x", data[i]);
  }
  printf("\r\n");
}
} // namespace stbox
