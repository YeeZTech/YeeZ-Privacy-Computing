#include <gtest/gtest.h>
#include <hpda/hpda.h>

define_nt(hash, std::string);
define_nt(value, uint64_t);
typedef hpda::ntobject<hash, value> data_item_t;

TEST(concat, basic) {
  hpda::extractor::raw_data<hash, value> rd1;
  hpda::extractor::raw_data<hash, value> rd2;

  hpda::engine engine;
  rd1.set_engine(&engine);
  rd2.set_engine(&engine);

  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(i), i);
    rd1.add_data(d);
    rd2.add_data(d);
  }
  hpda::processor::concat<hash, value> con(&rd1);
  con.add_upper_stream(&rd2);

  hpda::processor::filter<hash, value> f(
      &con, [](const data_item_t &d) { return d.get<value>() >= 500; });

  hpda::output::memory_output<hash, value> mo(&f);

  engine.run();

  auto s = mo.values().size();
  EXPECT_EQ(s, 1000);
}
