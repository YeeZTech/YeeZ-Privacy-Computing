#pragma once
#include <stdint.h>

extern "C" {
  void omove(uint32_t i, uint32_t *item, uint32_t loc, uint32_t *leaf, uint32_t newLabel);
  // void opush_real_block_into_stash();
  void oset_value(uint32_t *dest, uint32_t value, bool flag);
  void oset_flag(bool *dest_flag, bool src_flag, bool flag);
  void oset_bytes(uint8_t *dest_bytes, uint8_t *src_bytes, uint32_t bytes_size, bool flag);
}

