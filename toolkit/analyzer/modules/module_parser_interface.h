#pragma once
#include "ypc/core/byte.h"

class parser_module_interface {
public:
  virtual uint32_t begin_parse_data_item() = 0;
  virtual uint32_t parse_data_item(const uint8_t *data, size_t len) = 0;
  virtual uint32_t end_parse_data_item() = 0;

  virtual uint32_t get_enclave_hash(ypc::bytes &enclave_hash) = 0;

  virtual uint32_t get_analyze_result(ypc::bytes &res) = 0;

  virtual uint32_t init_data_source(const ypc::bytes &info) = 0;
  virtual uint32_t init_model(const ypc::bytes &info) = 0;
  virtual uint32_t get_parser_type() = 0;
};
