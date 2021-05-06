#include "./param_from_memory.h"

param_from_memory::param_from_memory(param_source &s) { copy_from(s); }

uint32_t param_from_memory::read_from_source() { return 0; }
