#pragma once
#include "params/param_source.h"

class param_from_json : public param_source {

public:
  param_from_json(const std::string &path);
  virtual uint32_t read_from_source();

protected:
  std::string m_path;
};
