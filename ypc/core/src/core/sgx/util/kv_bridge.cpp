#include "ypc/core/sgx/util/kv_bridge.h"
#include "ypc/core/byte.h"
#include <unordered_map>
extern "C" {

uint8_t has_key_ocall(const uint8_t *key);

uint32_t remove_key_ocall(const uint8_t *key);

uint32_t write_to_storage_ocall(const uint8_t *key, const uint8_t *val,
                                std::size_t len);

uint32_t read_from_storage_ocall(const uint8_t *key, uint8_t *val,
                                 std::size_t data_size);
};

namespace ypc {
std::unordered_map<ypc::bytes, ypc::bytes> g_kv_storage;
void init_sgx_kv() {}
void shutdown_sgx_kv() {}
} // namespace ypc

// TODO we should put these into a k-v database, like level db
uint8_t has_key_ocall(const uint8_t *key) {
  auto k =
      ypc::g_kv_storage.find(ypc::bytes(key, 32)) != ypc::g_kv_storage.end();
  return static_cast<uint8_t>(k);
}

uint32_t remove_key_ocall(const uint8_t *key) {
  ypc::g_kv_storage.erase(ypc::bytes(key, 32));
  return 0;
}

uint32_t write_to_storage_ocall(const uint8_t *key, const uint8_t *val,
                                std::size_t len) {

  auto k = ypc::bytes(key, 32);
  auto v = ypc::bytes(val, len);
  auto it = ypc::g_kv_storage.find(k);
  if (it == ypc::g_kv_storage.end()) {
    ypc::g_kv_storage.insert(std::make_pair(k, v));
  } else {
    it->second = v;
  }
  return 0;
}

uint32_t read_from_storage_ocall(const uint8_t *key, uint8_t *val,
                                 std::size_t data_size) {
  auto k = ypc::bytes(key, 32);
  auto it = ypc::g_kv_storage.find(k);
  if (it == ypc::g_kv_storage.end()) {
    return 1;
  }
  if (data_size != it->second.size()) {
    return 2;
  }
  memcpy(val, it->second.data(), data_size);
  return 0;
}
