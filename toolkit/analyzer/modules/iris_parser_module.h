#pragma once
#include "ypc/core/sgx/parser_sgx_module.h"

class iris_parser_module : public ypc::parser_sgx_module {
public:
  explicit iris_parser_module(const char *mod_path);
};
