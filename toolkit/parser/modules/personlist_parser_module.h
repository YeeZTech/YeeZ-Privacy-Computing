#pragma once
#include "ypc/core/byte.h"
#include "ypc/core/sgx/parser_sgx_module.h"

class personlist_parser_module : public ypc::parser_sgx_module {
public:
  explicit personlist_parser_module(const char *mod_path);

  personlist_parser_module(const personlist_parser_module &) = delete;
  personlist_parser_module(personlist_parser_module &&) = delete;
  personlist_parser_module &operator=(personlist_parser_module &&) = delete;
  personlist_parser_module &
  operator=(const personlist_parser_module &) = delete;

  virtual ~personlist_parser_module() = default;
};
