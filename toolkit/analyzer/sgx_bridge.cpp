#include "sgx_bridge.h"

using stx_status = stbox::stx_status;
std::shared_ptr<parser> g_parser;

extern "C" {
uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len);
}

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len) {
  return g_parser->next_data_batch(data_hash, hash_size, data, len);
}
