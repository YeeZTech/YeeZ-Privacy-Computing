#pragma once
#include <ff/util/ntobject.h>
#include <string>

define_nt(date, std::string);
define_nt(county, std::string);
define_nt(state, std::string);
define_nt(fips, std::string);
define_nt(cases, uint32_t);
define_nt(deaths, uint32_t);

typedef ff::util::ntobject<date, county, state, fips, cases, deaths>
    us_covid19_item_t;
typedef us_covid19_item_t user_item_t;
