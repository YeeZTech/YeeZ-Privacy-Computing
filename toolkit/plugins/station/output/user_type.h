#pragma once
#include <ff/util/ntobject.h>
#include <string>
define_nt(ID, uint64_t);
define_nt(CITY, std::string);
define_nt(STATE, std::string);
define_nt(LAT_N, float);
define_nt(LONG_W, float);
typedef ff::util::ntobject<ID,CITY,STATE,LAT_N,LONG_W> station_item_t;
