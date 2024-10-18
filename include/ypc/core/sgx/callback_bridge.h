#pragma once
#include "ypc/common/byte/bytes.h"
#include <functional>

namespace ypc {
using next_data_batch_func = std::function<uint32_t(const uint8_t *hash_and_pkey,
                                   uint32_t hash_and_pkey_size, uint8_t **data,
                                   uint32_t *len)>; // NOLINT
class callback_parser {
public:
  callback_parser(next_data_batch_func func)
      : m_next_data_batch_func(func) {}
  virtual ~callback_parser(){}


  inline uint32_t next_data_batch(const uint8_t *hash_and_pkey,
                                  uint32_t hash_and_pkey_size, uint8_t **data,
                                  uint32_t *len) {
    return m_next_data_batch_func(hash_and_pkey, hash_and_pkey_size, data, len);
  }

private:
  // std::shared_ptr<ParserModule> m_parser;
  next_data_batch_func m_next_data_batch_func;
};
void init_sgx_callback(next_data_batch_func func);
void shutdown_sgx_km();
}