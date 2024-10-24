#pragma once
#include "module_parser_interface.h"
#include "ypc/core/byte.h"
#include "ypc/stbox/usgx/sgx_module.h"

class iris_parser_module : public stbox::sgx_module,
                           public parser_module_interface {
public:
  explicit iris_parser_module(const char *mod_path);

  iris_parser_module(const iris_parser_module &) = delete;
  iris_parser_module(iris_parser_module &&) = delete;
  iris_parser_module &operator=(iris_parser_module &&) = delete;
  iris_parser_module &operator=(const iris_parser_module &) = delete;

  virtual ~iris_parser_module() = default;

  uint32_t begin_parse_data_item();
  uint32_t parse_data_item(const uint8_t *data, size_t len);
  uint32_t end_parse_data_item();

  uint32_t get_enclave_hash(ypc::bytes &enclave_hash);

  uint32_t get_analyze_result(ypc::bytes &res);

  uint32_t init_data_source(const ypc::bytes &info);
  uint32_t init_model(const ypc::bytes &info);
  uint32_t get_parser_type();
};
