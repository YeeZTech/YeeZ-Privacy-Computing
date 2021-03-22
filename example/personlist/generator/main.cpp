#include "../common.h"
#include <iostream>

// typedef ff::sql::table<ff::sql::mysql<ff::sql::cppconn>, person_list_meta,
// RYXXBZ, XM, CYM, XBDM, FWXXBZ, XP, DWMC, ZJHM, GJDM,
// MZDM, JGSSXDM, HKXZFLYDM, HLXDM, HJDZ_XZQHDM,
// SJJZD_XZQHDM, SJJZD_QHNXXDZ, XLDM, TSSFDM, CSQR, LXDH,
// HYZKDM, DJR_XM, DJR_GMSFZHM, DJR_LXDH, GXSJ, SJZT>
// person_list_table_t;

// typedef typename person_list_table_t::row_collection_type::row_type row_t;

row_t create(const std::string &id) {
  row_t t;

  t.set<RYXXBZ>("4028f c 856517466 f 0165174f 03680103");
  t.set<XM>("张三");
  t.set<FWXXBZ>("4028f c856517466 f 0165174f 03680105");
  t.set<ZJHM>(id);
  t.set<GJDM>("156");
  t.set<MZDM>("01");
  t.set<MZDM>("01");
  t.set<JGSSXDM>("420500");
  t.set<HKXZFLYDM>("10");
  t.set<HLXDM>("02");
  t.set<HJDZ_XZQHDM>("420500");
  t.set<HJDZ_XZQHDM>("XX省直昌市城申半岛");
  t.set<SJJZD_XZQHDM>("420500");
  t.set<SJJZD_QHNXXDZ>("XX省宜昌市域中 半岛");
  t.set<XLDM>("20");
  t.set<CSQR>(id);
  t.set<LXDH>("13800000000");
  t.set<HYZKDM>("10");
  t.set<DJR_XM>("张三");
  t.set<DJR_LXDH>("13800000000");
  t.set<GXSJ>("20200701170500");

  return t;
}

/*
void write_to_database() {
  ff::sql::mysql<ff::sql::cppconn> engine("localhost", "test", "1234",
                                          "testdb");
  person_list_table_t::create_table(&engine);
  person_list_table_t::row_collection_type rs;
  rs.push_back(create("19860726"));

  person_list_table_t::insert_or_replace_rows(&engine, rs);
}*/

void write_to_file(const std::string &path, int num) {
  uint64_t id = 421003198607262336;
  file_t f;
  f.open_for_write(path.c_str());

  for (int i = 0; i < num; i++) {
    row_t t = create(std::to_string(id + i));
    ff::net::marshaler lr(ff::net::marshaler::length_retriver);
    t.arch(lr);
    size_t len = lr.get_length();
    char *buf = new char[len];
    ff::net::marshaler m(buf, len, ff::net::marshaler::seralizer);
    t.arch(m);
    f.append_item(buf, len);
    delete[] buf;
  }
  f.close();
}

void check_file(const std::string &path) {

  file_t f;
  f.open_for_read(path.c_str());
  ypc::memref r;
  uint64_t id = 421003198607262336;
  int i = 0;
  while (f.next_item(r)) {

    ff::net::marshaler m(r.data(), r.len(), ff::net::marshaler::deseralizer);
    row_t t;
    t.arch(m);
    std::cout << t.get<ZJHM>() << std::endl;
    r.dealloc();
    i++;
  }
  std::cout << "checked " << i << " items" << std::endl;
}

int main(int argc, char *argv[]) {
  write_to_file("person_list", 1024 * 8);
  check_file("person_list");
  return 0;
}
