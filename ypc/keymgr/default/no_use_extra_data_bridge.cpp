#include <cstdint>
extern "C" {
uint32_t ocall_read_next_extra_data_item(uint8_t *data_hash,
                                         uint32_t hash_size);
uint32_t ocall_get_next_extra_data_item_size();
uint32_t ocall_get_next_extra_data_item_data(uint8_t *item_data,
                                             uint32_t ndi_size);
}

uint32_t ocall_read_next_extra_data_item(uint8_t *data_hash,
                                         uint32_t hash_size) {
  return 0;
}
uint32_t ocall_get_next_extra_data_item_size() { return 0; }
uint32_t ocall_get_next_extra_data_item_data(uint8_t *item_data,
                                             uint32_t ndi_size) {
  return 0;
}
