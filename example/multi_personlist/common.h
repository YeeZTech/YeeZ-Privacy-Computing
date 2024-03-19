#pragma once
#include "common_t.h"
#include "ypc/core/blockfile.h"
typedef ypc::blockfile<0x82, 2, 1024 * 1024, 256 * 64 * 1024> file_t;
