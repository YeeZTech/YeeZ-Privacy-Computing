#include <gtest/gtest.h>
#include <hpda/hpda.h>

define_nt(key1, std::string);
define_nt(key2, std::string);
define_nt(key3, std::string);
define_nt(value, uint32_t);
typedef hpda::ntobject<key1, value> data1_item_t;
typedef hpda::ntobject<key2, value> data2_item_t;
typedef hpda::ntobject<key3, value> data3_item_t;

TEST(intersection, ordered) {
  hpda::extractor::raw_data<key1, value> rd1;
  hpda::extractor::raw_data<key2, value> rd2;
  hpda::extractor::raw_data<key3, value> rd3;

  hpda::engine engine;
  rd1.set_engine(&engine);
  rd2.set_engine(&engine);
  rd3.set_engine(&engine);

  std::vector<std::string> vk1;
  for (int i = 0; i < 100; i++) {
    vk1.push_back(std::to_string(i));
  }
  std::sort(
      vk1.begin(), vk1.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk1) {
    data1_item_t d1;
    d1.set<key1, value>(k, std::stoi(k));
    rd1.add_data(d1);
  }

  std::vector<std::string> vk2;
  for (int i = 70; i < 170; i++) {
    vk2.push_back(std::to_string(i));
  }
  std::sort(
      vk2.begin(), vk2.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk2) {
    data2_item_t d2;
    d2.set<key2, value>(k, std::stoi(k));
    rd2.add_data(d2);
  }

  std::vector<std::string> vk3;
  for (int i = 90; i < 120; i++) {
    vk3.push_back(std::to_string(i));
  }
  std::sort(
      vk3.begin(), vk3.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk3) {
    data3_item_t d3;
    d3.set<key3, value>(k, std::stoi(k));
    rd3.add_data(d3);
  }

  hpda::processor::ordered_intersection<std::string, key1, key2, key3, value>
      psi;
  psi.add_upper_stream<key1>(&rd1);
  psi.add_upper_stream<key2>(&rd2);
  psi.add_upper_stream<key3>(&rd3);

  hpda::output::memory_output<key1, key2, key3, value> mo(&psi);

  engine.run();

  int index = 90;
  for (auto &it : mo.values()) {
    EXPECT_EQ(it.template get<key1>(), std::to_string(index));
    EXPECT_EQ(it.template get<value>(), index);
    index++;
  }
}

TEST(unionset, ordered) {
  hpda::extractor::raw_data<key1, value> rd1;
  hpda::extractor::raw_data<key2, value> rd2;
  hpda::extractor::raw_data<key3, value> rd3;

  hpda::engine engine;
  rd1.set_engine(&engine);
  rd2.set_engine(&engine);
  rd3.set_engine(&engine);

  std::vector<std::string> vk1;
  for (int i = 0; i < 100; i++) {
    vk1.push_back(std::to_string(i));
  }
  std::sort(
      vk1.begin(), vk1.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk1) {
    data1_item_t d1;
    d1.set<key1, value>(k, std::stoi(k));
    rd1.add_data(d1);
  }

  std::vector<std::string> vk2;
  for (int i = 20; i < 120; i++) {
    vk2.push_back(std::to_string(i));
  }
  std::sort(
      vk2.begin(), vk2.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk2) {
    data2_item_t d2;
    d2.set<key2, value>(k, std::stoi(k));
    rd2.add_data(d2);
  }

  std::vector<std::string> vk3;
  for (int i = 80; i < 180; i++) {
    vk3.push_back(std::to_string(i));
  }
  std::sort(
      vk3.begin(), vk3.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk3) {
    data3_item_t d3;
    d3.set<key3, value>(k, std::stoi(k));
    rd3.add_data(d3);
  }

  hpda::processor::ordered_union<std::string, key1, key2, key3, value> psi;
  psi.add_upper_stream<key1>(&rd1);
  psi.add_upper_stream<key2>(&rd2);
  psi.add_upper_stream<key3>(&rd3);

  hpda::output::memory_output<key1, key2, key3, value> mo(&psi);

  engine.run();

  size_t count = 0;
  std::vector<uint32_t> v;
  for (auto &it : mo.values()) {
    count++;
    v.push_back(it.template get<value>());
  }
  EXPECT_EQ(count, 180);
  std::sort(v.begin(), v.end(),
            [](const uint32_t &i, const uint32_t &j) { return i < j; });
  uint32_t val = 0;
  for (auto &it : v) {
    EXPECT_EQ(val, it);
    val++;
  }
}

TEST(difference, ordered) {
  hpda::extractor::raw_data<key1, value> rd1;
  hpda::extractor::raw_data<key2, value> rd2;

  hpda::engine engine;
  rd1.set_engine(&engine);
  rd2.set_engine(&engine);

  std::vector<std::string> vk1;
  for (int i = 0; i < 100; i++) {
    vk1.push_back(std::to_string(i));
  }
  std::sort(
      vk1.begin(), vk1.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk1) {
    data1_item_t d1;
    d1.set<key1, value>(k, std::stoi(k));
    rd1.add_data(d1);
  }

  std::vector<std::string> vk2;
  for (int i = 10; i < 110; i++) {
    vk2.push_back(std::to_string(i));
  }
  std::sort(
      vk2.begin(), vk2.end(),
      [](const std::string &s1, const std::string &s2) { return s1 < s2; });
  for (auto &k : vk2) {
    data2_item_t d2;
    d2.set<key2, value>(k, std::stoi(k));
    rd2.add_data(d2);
  }

  hpda::processor::ordered_difference<std::string, key1, key2, value> psi;
  psi.add_upper_stream<key1>(&rd1);
  psi.add_upper_stream<key2>(&rd2);

  hpda::output::memory_output<key1, key2, value> mo(&psi);

  engine.run();

  int count = 0;
  for (auto &it : mo.values()) {
    EXPECT_EQ(count++, it.template get<value>());
  }
  EXPECT_EQ(count, 10);
}
