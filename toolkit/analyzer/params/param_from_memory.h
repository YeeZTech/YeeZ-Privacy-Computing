#pragma once
#include "./param_source.h"
class param_from_memory : public param_source {
public:
  param_from_memory(param_source &s);
  virtual uint32_t read_from_source();
};
