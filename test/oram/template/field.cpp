#include "field.h"
#include <iostream>

template<typename ObjType>
std::string get_feild(ObjType obj) {
  return obj.get<YM>();
}

std::string get_feild(row_t obj) {
  return obj.get<YM>();
}

int main() {
  row_t line;
  line.set<YM, XM>("jhjdhfjf", "hfuhue");
  // std::cout << get_feild<row_t, YM>(line) << std::endl;
  std::cout << get_feild(line) << std::endl;
  std::cout << get_feild<row_t>(line) << std::endl;
  std::cout << add<int>(1, 1) << std::endl;
  std::cout << add<double>(2.0, 3.2) << std::endl;
}