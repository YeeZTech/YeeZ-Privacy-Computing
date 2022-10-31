#pragma once
#include <cstddef>
#include <cstdint>

namespace stbox {
int printf(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
void print_hex(uint8_t *data, size_t data_len);
} // namespace stbox
