#include "ff/sql/mysql.hpp"
#include "ff/sql/table.h"
#include "toolkit/plugins/mysql/mysql_reader.h"
#include "ypc_t/analyzer/ntpackage_item_parser.h"
#include <iostream>

define_nt(v1, int);
define_nt(s1, std::string);
define_nt(v2, double);

define_column(cv1, key, int, "v1");
define_column(cs1, column, std::string, "s1");
define_column(cv2, column, double, "v2");

struct mymeta {
  constexpr static const char *table_name = "xxxyyy";
};
typedef ff::sql::table<ff::sql::mysql<ff::sql::cppconn>, mymeta, cv1, cs1, cv2>
    mytable;

typedef mytable::row_collection_type::row_type mt;

void generate_mysql_data(ff::sql::mysql<ff::sql::cppconn> &engine) {

  try {
    mytable::drop_table(&engine);
  } catch (...) {
    std::cout << "drop_table failed" << std::endl;
  }
  mytable::create_table(&engine);
  mytable::row_collection_type rows;
  mytable::row_collection_type::row_type t1;
  t1.set<cv1, cs1, cv2>(1, "test", 2.3f);
  rows.push_back(t1.make_copy());
  t1.set<cv1, cs1, cv2>(2, "test2", 2.4f);
  rows.push_back(t1.make_copy());
  t1.set<cv1, cs1, cv2>(3, "test3", 3.4f);
  rows.push_back(t1.make_copy());
  mytable::insert_or_replace_rows(&engine, rows);
}

int main(int argc, char *argv[]) {
  ff::sql::mysql<ff::sql::cppconn> engine("tcp://127.0.0.1:3306", argv[1],
                                          argv[2], "testdb");
  std::cout << "done init" << std::endl;
  generate_mysql_data(engine);
  // clang-format off
  std::stringstream ss;
  ss<<"{\"url\":\"tcp://127.0.0.1:3306\", \"username\":\""<<argv[1]<<"\", \"password\":\""<<argv[2]<<"\", \"dbname\":\"testdb\"}";
  std::string json =ss.str();
  // clang-format on
  std::cout << json << std::endl;
  ypc::plugins::typed_mysql_reader<mt, mytable> r(json);

  std::cout << "item number: " << r.get_item_number() << std::endl;
  int len;
  r.read_item_data(nullptr, &len);
  char *buf = new char[len];
  r.read_item_data(buf, &len);
  mt p = ypc::ntpackage_item_parser<char, mt>::parser(buf, len);
  std::cout << p.get<cv1>() << ", " << p.get<cs1>() << ", " << p.get<cv2>()
            << std::endl;

  delete[] buf;
  return 0;
}
