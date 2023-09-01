#include "../common.h"
#include "ypc/core/byte.h"
#include "ypc/corecommon/package.h"
#include <iostream>
#include <random>

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
  t.set<XM>("张三+" + id);
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

void write_to_file(const std::string &path, int num) {
  uint64_t id = 421003198607262336;
  file_t f;
  f.open_for_write(path.c_str());

  for (int i = 0; i < num; i++) {
    row_t t = create(std::to_string(id + i));
    typename ypc::cast_obj_to_package<row_t>::type pt = t;
    auto buf = ypc::make_bytes<ypc::bytes>::for_package(pt);
    f.append_item(buf.data(), buf.size());
  }
  f.close();
}

// row_t create(const std::string &id, const std::string &c_id) {
//   row_t t;

//   t.set<RYXXBZ>("4028f c 856517466 f 0165174f 03680103");
//   t.set<XM>(c_id + "+" + id);
//   t.set<FWXXBZ>("4028f c856517466 f 0165174f 03680105");
//   t.set<ZJHM>(id);
//   t.set<GJDM>("156");
//   t.set<MZDM>("01");
//   t.set<MZDM>("01");
//   t.set<JGSSXDM>("420500");
//   t.set<HKXZFLYDM>("10");
//   t.set<HLXDM>("02");
//   t.set<HJDZ_XZQHDM>("420500");
//   t.set<HJDZ_XZQHDM>("XX省直昌市城申半岛");
//   t.set<SJJZD_XZQHDM>("420500");
//   t.set<SJJZD_QHNXXDZ>("XX省宜昌市域中 半岛");
//   t.set<XLDM>("20");
//   t.set<CSQR>(id);
//   t.set<LXDH>("13800000000");
//   t.set<HYZKDM>("10");
//   t.set<DJR_XM>("张三");
//   t.set<DJR_LXDH>("13800000000");
//   t.set<GXSJ>("20200701170500");

//   return t;
// }

// void write_to_file(const std::string &path, int num) {
//   uint64_t id = 421003198607262336;
//   file_t f;
//   f.open_for_write(path.c_str());

//   static std::default_random_engine generator;
//   static std::uniform_int_distribution<int> distribution(1, 10);
//   static auto rand = std::bind(distribution, generator);

//   for (int i = 0; i < num; i++) {
//     int ran_num = rand();
//     row_t t = create(std::to_string(id + i), std::to_string(id + i));
//     typename ypc::cast_obj_to_package<row_t>::type pt = t;
//     auto buf = ypc::make_bytes<ypc::bytes>::for_package(pt);
//     f.append_item(buf.data(), buf.size());
//   }
//   f.close();
// }

void check_file(const std::string &path) {

  file_t f;
  f.open_for_read(path.c_str());
  ypc::memref r;
  uint64_t id = 421003198607262336;
  int i = 0;
  while (f.next_item(r)) {
    typedef typename ypc::cast_obj_to_package<row_t>::type pkg_t;
    auto pkg =
        ypc::make_package<pkg_t>::from_bytes(ypc::bytes(r.data(), r.size()));

    // std::cout << pkg.get<ZJHM>() << std::endl;
    i++;
  }
  std::cout << "checked " << i << " items" << std::endl;
}

int main(int argc, char *argv[]) {
  write_to_file("person_list_oram", 1 << 16);
  check_file("person_list_oram");
  return 0;
}
