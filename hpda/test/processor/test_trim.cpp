#include <gtest/gtest.h>
#include <hpda/hpda.h>

define_nt(hash, std::string);
define_nt(value, uint64_t);
typedef hpda::ntobject<hash, value> data_item_t;
typedef hpda::ntobject<value> value_item_t;

TEST(trim, basic) {
  hpda::extractor::raw_data<hash, value> rd;

  hpda::engine engine;
  rd.set_engine(&engine);

  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(i), i);
    rd.add_data(d);
  }

  hpda::processor::trim_t<data_item_t, value_item_t> f(&rd);
  hpda::output::memory_output<value> mo(&f);

  engine.run();
  auto &s = mo.values();
  EXPECT_EQ(s.size(), 1000);

  for (int i = 0; i < 1000; i++) {
    EXPECT_EQ(s[i].get<::value>(), i);
  }
}
