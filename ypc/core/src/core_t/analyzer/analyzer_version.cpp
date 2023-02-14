#include "ypc/core_t/analyzer/yaenclave_t_interface.h"
#include "ypc/version.h"
uint64_t get_ypc_analyzer_version() { return YPC_CORE_T_VERSION.data(); }
