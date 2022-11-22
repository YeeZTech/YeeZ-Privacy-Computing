#include "yaenclave_t.h"
#include "ypc/version.h"
uint64_t get_ypc_analyzer_version() { return YPC_CORE_T_VERSION.data(); }
