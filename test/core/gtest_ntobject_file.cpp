#include "ypc/ntobject_file.h"
#include <gtest/gtest.h>

define_nt(sfm_path, std::string);
define_nt(sfm_index, uint16_t);
define_nt(sfm_hash, std::string);

typedef ::ff::net::ntpackage<1, sfm_path, sfm_index, sfm_hash> sfm_item_t;
define_nt(sfm_items, std::vector<sfm_item_t>);

typedef ::ff::net::ntpackage<2, sfm_items, sfm_hash> sfm_t;

TEST(test_ntobject_file, ntpackage) {
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

typedef ::ff::util::ntobject<sfm_path, sfm_index, sfm_hash> sfm_obj_t;
define_nt(sfm_objs, std::vector<sfm_obj_t>);

typedef ::ff::util::ntobject<sfm_objs, sfm_hash> sfm_s_t;

TEST(test_ntobject_file, assign) {
  sfm_s_t data;
  sfm_obj_t item;
  item.set<sfm_path, sfm_index, sfm_hash>("test", 123, "xxoo");

  std::vector<sfm_obj_t> items;
  items.push_back(item);
  data.set<sfm_objs>(items);
  data.set<sfm_hash>("dxdx");

  sfm_s_t data2 = data;

  EXPECT_EQ(data2.get<sfm_objs>().size(), data.get<sfm_objs>().size());
  EXPECT_EQ(data2.get<sfm_hash>(), "dxdx");
  EXPECT_EQ(data2.get<sfm_objs>()[0].get<sfm_index>(), 123);
}

TEST(test_ntobject_file, ntobject) {
  ypc::ntobject_file<sfm_s_t> file("./test_ntobject_file_output2");
  sfm_s_t data;
  sfm_obj_t item;
  item.set<sfm_path, sfm_index, sfm_hash>("test test", 0xEEEE, "xxoo");

  std::vector<sfm_obj_t> items;

  items.push_back(item);
  data.set<sfm_objs>(items);
  data.set<sfm_hash>("abcd");

  // ff::net::marshaler m(ff::net::marshaler::length_retriver);
  // data.arch(m);
  file.data() = data;

  file.write_to();

  ypc::ntobject_file<sfm_s_t> rfile("./test_ntobject_file_output2");
  rfile.read_from();
  sfm_s_t data2 = rfile.data();
  item = data2.get<sfm_objs>()[0];
  std::cout << "item :" << item.get<sfm_index>() << std::endl;

  std::cout << "data2 : " << data2.get<sfm_hash>() << std::endl;

  EXPECT_EQ(rfile.data().get<sfm_objs>().size(), data.get<sfm_objs>().size());
  EXPECT_EQ(rfile.data().get<sfm_objs>()[0].get<sfm_index>(), 0xEEEE);
  EXPECT_EQ(rfile.data().get<sfm_hash>(), std::string("abcd"));
}
