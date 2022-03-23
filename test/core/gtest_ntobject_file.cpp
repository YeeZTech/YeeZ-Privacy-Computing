#include "ypc/ntobject_file.h"
#include <gtest/gtest.h>

define_nt(sfm_path, std::string);
define_nt(sfm_index, uint16_t);
define_nt(sfm_hash, std::string);
typedef ::ff::net::ntpackage<1, sfm_path, sfm_index, sfm_hash> sfm_item_t;
define_nt(sfm_items, std::vector<sfm_item_t>);

typedef ::ff::net::ntpackage<2, sfm_items, sfm_hash> sfm_t;

TEST(test_ntobject_file, simple) {
  ypc::ntobject_file<sfm_t> file("./test_ntobject_file_output");
  sfm_t data;
  sfm_item_t item;
  item.set<sfm_path, sfm_index, sfm_hash>("test", 123, "xxoo");

  std::vector<sfm_item_t> items;
  items.push_back(item);
  data.set<sfm_items>(items);
  data.set<sfm_hash>("你好");

  ff::net::marshaler m(ff::net::marshaler::length_retriver);
  data.arch(m);
  file.data() = data;

  file.write_to();

  ypc::ntobject_file<sfm_t> rfile("./test_ntobject_file_output");
  rfile.read_from();

  EXPECT_EQ(rfile.data().get<sfm_items>().size(), data.get<sfm_items>().size());
  EXPECT_EQ(rfile.data().get<sfm_hash>(), "你好");
  EXPECT_EQ(rfile.data().get<sfm_items>()[0].get<sfm_index>(), 123);
}
