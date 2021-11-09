#pragma once
#include "corecommon/nt_cols.h"
#include "ypc/byte.h"
#include "ypc/ref.h"

typedef ypc::nt<ypc::bytes>::ypc_result_package_t result_pkg_t;

class result_target {
public:
  virtual void write_to_target(const result_pkg_t &res) = 0;

  virtual void read_from_target(ypc::bytes &encrypted_result,
                                ypc::bytes &result_signature,
                                ypc::bytes &cost_signature,
                                ypc::bytes &data_hash) = 0;
};
