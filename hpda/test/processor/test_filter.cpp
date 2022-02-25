#include <gtest/gtest.h>
#include <hpda/hpda.h>

define_nt(hash, std::string);
define_nt(value, uint64_t);
typedef hpda::ntobject<hash, value> data_item_t;

TEST(filter, basic) {
  hpda::extractor::raw_data<hash, value> rd;

  hpda::engine engine;
  rd.set_engine(&engine);

  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(i), i);
    rd.add_data(d);
  }

  hpda::processor::filter<hash, value> f(&rd, [](const data_item_t &d) {
    return d.get<value>() >= 500;
  });

  hpda::output::memory_output<hash, value> mo(&f);

  engine.run();
  auto s = mo.values().size();
  EXPECT_EQ(s, 500);
}
