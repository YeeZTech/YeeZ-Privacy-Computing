#pragma once
#include "params/param_source.h"

class param_from_json : public param_source {

public:
  param_from_json(const std::string &path);
  virtual void read_from_source();

protected:
  std::string m_path;
};
