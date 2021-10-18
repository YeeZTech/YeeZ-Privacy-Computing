#include "toolkit/plugins/csv/csv_reader.h"
#include "ypc_t/analyzer/ntpackage_item_parser.h"
#include <iostream>

define_nt(v1, int);
define_nt(s1, std::string);
define_nt(v2, double);
typedef ::ff::util::ntobject<v1, s1, v2> mt;


int main(int argc, char *argv[]) {
  // clang-format off
  std::string json("{\"file_path\":\"../test/toolkit/plugins/csv/test1.csv\"}");
  // clang-format on
  std::cout << json << std::endl;
  ypc::plugins::typed_csv_reader<mt> r(json);

  std::cout << r.get_item_number() << std::endl;
  int len;
  r.read_item_data(nullptr, &len);
  char *buf = new char[len];
  r.read_item_data(buf, &len);
  mt p = ypc::ntpackage_item_parser<char, mt>::parser(buf, len);
  std::cout << p.get<v1>() << ", " << p.get<s1>() << ", " << p.get<v2>()
            << std::endl;

  delete[] buf;
  return 0;
}
