#pragma once
#include "result/result_target.h"

class result_to_json : public result_target {

public:
  result_to_json(const std::string &path);

  virtual void write_to_target(const ypc::bref &encrypted_result,
                               const ypc::bref &result_signature,
                               const ypc::bref &cost_signature,
                               const ypc::bref &data_hash);
  virtual void read_from_target(ypc::bytes &encrypted_result,
                                ypc::bytes &result_signature,
                                ypc::bytes &cost_signature,
                                ypc::bytes &data_hash);

protected:
  std::string m_path;
};
