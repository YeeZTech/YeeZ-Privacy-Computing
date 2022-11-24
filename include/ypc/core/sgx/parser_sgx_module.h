#pragma once
#include "sgx_eid.h"
#include "sgx_error.h"
#include "ypc/common/parser_type.h"
#include "ypc/core/byte.h"
#include "ypc/core/ref.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/stbox/usgx/sgx_module.h"
#include <string>

using stx_status = stbox::stx_status;
using parser_type_t = ypc::utc::parser_type_t;
namespace ypc {
class parser_sgx_module : public stbox::sgx_module {
public:
  parser_sgx_module(const char *mod_path);
  virtual ~parser_sgx_module();

  uint32_t init();
  uint32_t init_parser();
  uint32_t parse_data_item(const uint8_t *data, size_t len);
  uint32_t finalize();

  uint32_t get_enclave_hash(ypc::bytes &enclave_hash);

  uint32_t get_analyze_result(ypc::bytes &res);


  uint32_t init_data_source(const ypc::bytes &info);
  uint32_t init_model(const ypc::bytes &info);
  uint32_t get_parser_type();
};
} // namespace ypc
