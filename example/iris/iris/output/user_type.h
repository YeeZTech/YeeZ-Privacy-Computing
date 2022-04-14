#pragma once
#include <ff/util/ntobject.h>
#include <string>
define_nt(sepal_len, double);
define_nt(sepal_wid, double);
define_nt(petal_len, double);
define_nt(petal_wid, double);
define_nt(species, std::string);
typedef ff::util::ntobject<sepal_len,sepal_wid,petal_len,petal_wid,species> iris_item_t;
