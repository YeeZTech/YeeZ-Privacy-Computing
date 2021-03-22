#pragma once
#include "ypc/byte.h"
#include "ypc/ref.h"

class result_target {
public:
  virtual void write_to_target(const ypc::bref &encrypted_result,
                               const ypc::bref &result_signature,
                               const ypc::bref &data_hash) = 0;

  virtual void read_from_target(ypc::bytes &encrypted_result,
                                ypc::bytes &result_signature,
                                ypc::bytes &data_hash) = 0;
};
