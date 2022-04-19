#include "ypc/ntjson.h"
#include "ypc/ntobject_file.h"
#include <gtest/gtest.h>

define_nt(sfm_path, std::string, "sfm-path");
define_nt(sfm_index, uint16_t);
define_nt(sfm_hash, std::string);
define_nt(sfm_bytes, ypc::bytes);
define_nt(sfm_double, double);
define_nt(sfm_values, std::vector<double>);

typedef ff::util::ntobject<sfm_path, sfm_index, sfm_hash, sfm_bytes> sfm_obj_t;
typedef ff::util::ntobject<sfm_path, sfm_index, sfm_hash, sfm_bytes, sfm_double>
    sfm_objd_t;

TEST(test_ntjson, simple) {
  sfm_obj_t t;
  t.set<sfm_path>("path here");
  t.set<sfm_index, sfm_hash, sfm_bytes>(
      123, "abcd", ypc::hex_bytes("abcd").as<ypc::bytes>());
  std::string json = ypc::ntjson::to_json(t);
  sfm_obj_t t2 = ypc::ntjson::from_json<sfm_obj_t>(json);
  EXPECT_TRUE(t.get<sfm_path>() == t2.get<sfm_path>());
  EXPECT_TRUE(t.get<sfm_index>() == t2.get<sfm_index>());
  EXPECT_TRUE(t.get<sfm_hash>() == t2.get<sfm_hash>());
  EXPECT_TRUE(t.get<sfm_bytes>() == t2.get<sfm_bytes>());
}

TEST(test_ntjson, file) {
  sfm_obj_t t;
  t.set<sfm_path>("path here");
  t.set<sfm_index, sfm_hash, sfm_bytes>(
      123, "abcd", ypc::hex_bytes("abcd").as<ypc::bytes>());
  ypc::ntjson::to_json_file(t, "test_ntjson.json");
  sfm_obj_t t2 = ypc::ntjson::from_json_file<sfm_obj_t>("test_ntjson.json");
  EXPECT_TRUE(t.get<sfm_path>() == t2.get<sfm_path>());
  EXPECT_TRUE(t.get<sfm_index>() == t2.get<sfm_index>());
  EXPECT_TRUE(t.get<sfm_hash>() == t2.get<sfm_hash>());
  EXPECT_TRUE(t.get<sfm_bytes>() == t2.get<sfm_bytes>());
}

TEST(test_ntjson, read_less) {
  sfm_obj_t t;
  t.set<sfm_path>("path here");
  t.set<sfm_index, sfm_hash, sfm_bytes>(
      123, "abcd", ypc::hex_bytes("abcd").as<ypc::bytes>());
  std::string json = ypc::ntjson::to_json(t);
  auto t2 = ypc::ntjson::from_json<sfm_objd_t>(json);
  EXPECT_TRUE(t.get<sfm_path>() == t2.get<sfm_path>());
  EXPECT_TRUE(t.get<sfm_index>() == t2.get<sfm_index>());
  EXPECT_TRUE(t.get<sfm_hash>() == t2.get<sfm_hash>());
  EXPECT_TRUE(t.get<sfm_bytes>() == t2.get<sfm_bytes>());
}

typedef ff::util::ntobject<sfm_path, sfm_index, sfm_hash, sfm_bytes, sfm_values>
    sfm_obj_values_t;

TEST(test_ntjson, vector_with_basic_value) {
  sfm_obj_values_t t;
  t.set<sfm_path>("path here");
  t.set<sfm_index, sfm_hash, sfm_bytes>(
      123, "abcd", ypc::hex_bytes("abcd").as<ypc::bytes>());
  std::vector<double> values;
  values.push_back(1.2);
  values.push_back(1.3);
  t.set<sfm_values>(values);
  std::string json = ypc::ntjson::to_json(t);
  std::cout << "json: " << json << std::endl;
  auto t2 = ypc::ntjson::from_json<sfm_obj_values_t>(json);
  std::string json2 = ypc::ntjson::to_json(t2);
  std::cout << "json2: " << json2 << std::endl;
  EXPECT_TRUE(t.get<sfm_path>() == t2.get<sfm_path>());
  EXPECT_TRUE(t.get<sfm_index>() == t2.get<sfm_index>());
  EXPECT_TRUE(t.get<sfm_hash>() == t2.get<sfm_hash>());
  EXPECT_TRUE(t.get<sfm_bytes>() == t2.get<sfm_bytes>());
  EXPECT_TRUE(t.get<sfm_values>().size() == t2.get<sfm_values>().size());
}
typedef ff::util::ntobject<sfm_hash, sfm_bytes> obj_t;
define_nt(sfm_hashes, std::vector<obj_t>);

typedef ff::util::ntobject<sfm_path, sfm_index, sfm_hashes> sfm_obj_hashes_t;

TEST(test_ntjson, vector_with_ntobj_value) {
  sfm_obj_hashes_t t;
  t.set<sfm_path, sfm_index>("path here", 123);
  std::vector<obj_t> values;
  {
    obj_t t;
    t.set<sfm_hash, sfm_bytes>("abcd", ypc::hex_bytes("abcd").as<ypc::bytes>());
    values.push_back(t);
  }

  {
    obj_t t;
    t.set<sfm_hash, sfm_bytes>("abcdef",
                               ypc::hex_bytes("abcdef").as<ypc::bytes>());
    values.push_back(t);
  }
  t.set<sfm_hashes>(values);
  std::string json = ypc::ntjson::to_json(t);
  auto t2 = ypc::ntjson::from_json<sfm_obj_hashes_t>(json);
  std::string json2 = ypc::ntjson::to_json(t2);
  EXPECT_TRUE(t.get<sfm_path>() == t2.get<sfm_path>());
  EXPECT_TRUE(t.get<sfm_index>() == t2.get<sfm_index>());
  EXPECT_TRUE(json == json2);
}
define_nt(sfm_object, obj_t);

typedef ff::util::ntobject<sfm_path, sfm_index, sfm_object> sfm_object_t;
TEST(test_ntjson, nested_obj_value) {
  sfm_object_t t;
  t.set<sfm_path, sfm_index>("path here", 123);
  obj_t ot;
  ot.set<sfm_hash, sfm_bytes>("abcd", ypc::hex_bytes("abcd").as<ypc::bytes>());
  t.set<sfm_object>(ot);

  std::string json = ypc::ntjson::to_json(t);
  auto t2 = ypc::ntjson::from_json<sfm_object_t>(json);
  std::string json2 = ypc::ntjson::to_json(t2);
  std::cout << "json :" << json2 << std::endl;
  EXPECT_TRUE(t.get<sfm_path>() == t2.get<sfm_path>());
  EXPECT_TRUE(t.get<sfm_index>() == t2.get<sfm_index>());
  EXPECT_TRUE(json == json2);
}
