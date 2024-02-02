#include <string>
#include "ypc/corecommon/package.h"
#include <ff/util/ntobject.h>

define_nt(YM, std::string, "YM");
define_nt(XM, std::string, "XM");

typedef ff::util::ntobject<
    YM, XM>
    row_t;

// template<typename ObjType, typename FieldName>
// std::string get_feild(ObjType obj) {
//   return obj.get<FieldName>();
// }



template<typename T>
T add(T a, T b) {
  return a + b;
}