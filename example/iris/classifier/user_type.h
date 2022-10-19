#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>
#include <string>

define_nt(sepal_len, double);
define_nt(sepal_wid, double);
define_nt(petal_len, double);
define_nt(petal_wid, double);
define_nt(species, std::string);

typedef ff::net::ntpackage<0, sepal_len, sepal_wid, petal_len, petal_wid>
    iris_data_t;
define_nt(iris_data, iris_data_t);

typedef ff::util::ntobject<iris_data, species> iris_item_t;
typedef iris_item_t user_item_t;

