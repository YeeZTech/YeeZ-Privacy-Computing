#pragma once
#include <ff/util/ntobject.h>
#include <string>
define_nt(ID, uint64_t);
define_nt(MONTH, uint64_t);
define_nt(TEMP_F, float);
define_nt(RAIN_I, float);
typedef ff::util::ntobject<ID,MONTH,TEMP_F,RAIN_I> stats_item_t;
