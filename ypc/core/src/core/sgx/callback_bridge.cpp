#include "ypc/core/sgx/callback_bridge.h"
#include "eparser_u.h"
#include <glog/logging.h>
#include <memory>

extern "C" {
uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len);
}

namespace ypc {
    std::shared_ptr<ypc::callback_parser> cb_parser;
    void init_sgx_callback(next_data_batch_func func) {
       cb_parser = std::make_shared<ypc::callback_parser>(func);
    }
    void shutdown_sgx_km() {}
} // namespace ypc


uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len) {
  return ypc::cb_parser->next_data_batch(data_hash, hash_size, data, len);
}
