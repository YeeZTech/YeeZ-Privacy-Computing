#pragma once
#include "ypc/core/sgx/parser_sgx_module.h"

class person_parser_module : public ypc::parser_sgx_module {
public:
  explicit person_parser_module(const char *mod_path);
};
