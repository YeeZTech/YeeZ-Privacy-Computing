#pragma once
#include "ypc/core/byte.h"
#include "ypc/core/sgx/parser_sgx_module.h"

class iris_parser_module : public ypc::parser_sgx_module {
public:
  explicit iris_parser_module(const char *mod_path);

  iris_parser_module(const iris_parser_module &) = delete;
  iris_parser_module(iris_parser_module &&) = delete;
  iris_parser_module &operator=(iris_parser_module &&) = delete;
  iris_parser_module &operator=(const iris_parser_module &) = delete;

  virtual ~iris_parser_module() = default;
};
