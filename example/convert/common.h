#pragma once
#include "common_t.h"
#include "ypc/core/blockfile.h"
#include "ypc/core/byte.h"
#include "ypc/corecommon/package.h"
typedef ypc::blockfile<0x82, 1024 * 1024, 256 * 64 * 1024> file_t;

