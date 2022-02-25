#pragma once
#include "./extra_data_source.h"
#include "ypc/sealed_file.h"
#include <unordered_map>

class extra_data_source_reader {
public:
  extra_data_source_reader(const ypc::extra_data_source_t &eds);
  uint32_t next_extra_data_item(const uint8_t *data_hash,
                                uint32_t data_hash_size);

  uint32_t get_next_extra_data_item_size();
  uint32_t get_next_extra_data_item_data(uint8_t *item_data, uint32_t ndi_size);

protected:
  std::string find_file_path_for_data(const uint8_t *data_hash,
                                      uint32_t hash_size);

  const ypc::extra_data_source_t m_data_source;
  std::shared_ptr<ypc::simple_sealed_file> m_current_file;
  ypc::bytes m_current_data_hash;
  bool m_has_value;
  ypc::memref m_data;
};
