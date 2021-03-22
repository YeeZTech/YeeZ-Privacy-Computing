#pragma once
#include "./param_source.h"
class param_from_memory : public param_source {
public:
  param_from_memory(param_source &s);
  virtual void read_from_source();
};
