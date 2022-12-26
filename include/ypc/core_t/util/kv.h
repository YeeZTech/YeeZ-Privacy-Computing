#pragma once
#include <cstdint>
#include <cstdio>

namespace ypc {
namespace kv {

// key length must be 32
bool has_key(const uint8_t *key);
uint32_t remove_key(const uint8_t *key);
uint32_t write(const uint8_t *key, const uint8_t *val, size_t len);
uint32_t read(const uint8_t *key, uint8_t *val, size_t size);
} // namespace kv
} // namespace ypc
