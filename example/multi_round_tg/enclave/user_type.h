#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>
#include <string>

define_nt(id, std::string);
define_nt(shxydm, std::string);
define_nt(qycm, std::string);
define_nt(hylx, std::string);
define_nt(qylx, std::string);
define_nt(jxmc, std::string);
define_nt(zcdz, std::string);
define_nt(jydz, std::string);
typedef ff::util::ntobject<id, shxydm, qycm, hylx, qylx, jxmc, zcdz, jydz>
    t_org_info_item_t;

define_nt(tax, float);
define_nt(qjsr, float);
define_nt(year, std::string);
typedef ff::util::ntobject<id, shxydm, tax, qjsr, year> t_tax_item_t;

typedef ff::util::ntobject<id, shxydm, qycm, hylx, qylx, jxmc, zcdz, jydz, tax,
                           qjsr, year>
    merged_item_t;
