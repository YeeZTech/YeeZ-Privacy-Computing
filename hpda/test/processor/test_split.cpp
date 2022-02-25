#include <gtest/gtest.h>
#include <hpda/hpda.h>

define_nt(hash, std::string);
define_nt(value, uint64_t);
typedef hpda::ntobject<hash, value> data_item_t;

TEST(split, basic) {
  hpda::extractor::raw_data<hash, value> rd;

  hpda::engine engine;
  rd.set_engine(&engine);

  for (int i = 0; i < 10; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(i), i);
    rd.add_data(d);
  }

  hpda::processor::split<hash, value> split(&rd);

  hpda::output::memory_output<hash, value> mo1(split.new_split_stream());
  hpda::output::memory_output<hash, value> mo2(split.new_split_stream());

  engine.run();
  auto s = mo1.values().size();
  EXPECT_EQ(s, 10);
  EXPECT_EQ(s, mo2.values().size());
}
