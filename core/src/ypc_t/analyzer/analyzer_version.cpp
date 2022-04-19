#include "yaenclave_t.h"
#include "ypc_t/ecommon/version.h"
uint32_t get_ypc_analyzer_version() { return ypc::version(1, 0, 0).data(); }
