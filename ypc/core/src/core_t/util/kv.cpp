#include "ypc/core_t/util/kv.h"
#include "eparser_t.h"
#include "ypc/stbox/tsgx/ocall.h"
namespace ypc {
namespace kv {

// key length must be 32
bool has_key(const uint8_t *key) {
  return stbox::ocall_cast<uint8_t>(has_key_ocall)(key) != 0;
}
uint32_t remove_key(const uint8_t *key) {
  return stbox::ocall_cast<uint32_t>(remove_key_ocall)(key);
}
uint32_t write(const uint8_t *key, const uint8_t *val, std::size_t len) {
  return stbox::ocall_cast<uint32_t>(write_to_storage_ocall)(key, val, len);
}
uint32_t read(const uint8_t *key, uint8_t *val, std::size_t size) {
  return stbox::ocall_cast<uint32_t>(read_from_storage_ocall)(key, val, size);
}
} // namespace kv
} // namespace ypc
