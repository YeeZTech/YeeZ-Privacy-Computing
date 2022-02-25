#include <gtest/gtest.h>
#include <hpda/hpda.h>

define_nt(hash, std::string);
define_nt(value, uint64_t);
define_nt(sum_v, uint64_t);
typedef hpda::ntobject<hash, value> data_item_t;

TEST(groupby, sum) {
  hpda::extractor::raw_data<hash, value> rd;
  hpda::engine engine;
  rd.set_engine(&engine);
  int sum = 0;
  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(100), i);
    rd.add_data(d);
    sum += i;
  }
  int sum2 = 0;
  for (int i = 0; i < 10; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(10), i);
    rd.add_data(d);
    sum2 += i;
  }

  typedef hpda::processor::group::sum<value, sum_v> sum_t;
  sum_t sum_aggregator;
  hpda::processor::groupby<hpda::extractor::raw_data<hash, value>::output_type,
                           hash, sum_t, hash, sum_v>
      f(&rd, sum_aggregator);

  hpda::output::memory_output<hash, sum_v> mo(&f);

  engine.run();
  auto s = mo.values().size();
  EXPECT_EQ(s, 2);
  EXPECT_EQ(mo.values()[0].get<sum_v>(), sum);
  EXPECT_EQ(mo.values()[1].get<sum_v>(), sum2);
}

TEST(groupby, avg) {
  hpda::extractor::raw_data<hash, value> rd;
  hpda::engine engine;
  rd.set_engine(&engine);
  int sum = 0;
  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(100), i);
    rd.add_data(d);
    sum += i;
  }
  int sum2 = 0;
  for (int i = 0; i < 10; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(10), i);
    rd.add_data(d);
    sum2 += i;
  }

  typedef hpda::processor::group::avg<value, sum_v> avg_t;
  avg_t aggregator;
  hpda::processor::groupby<hpda::extractor::raw_data<hash, value>::output_type,
                           hash, avg_t, hash, sum_v>
      f(&rd, aggregator);

  hpda::output::memory_output<hash, sum_v> mo(&f);

  engine.run();
  auto s = mo.values().size();
  EXPECT_EQ(s, 2);
  EXPECT_EQ(mo.values()[0].get<sum_v>(), sum / 1000);
  EXPECT_EQ(mo.values()[1].get<sum_v>(), sum2 / 10);
}

TEST(groupby, max) {
  hpda::extractor::raw_data<hash, value> rd;
  hpda::engine engine;
  rd.set_engine(&engine);
  int sum = 0;
  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(100), i);
    rd.add_data(d);
    sum += i;
  }
  int sum2 = 0;
  for (int i = 0; i < 10; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(10), i);
    rd.add_data(d);
    sum2 += i;
  }

  typedef hpda::processor::group::max<value, sum_v> max_t;
  max_t aggregator;
  hpda::processor::groupby<hpda::extractor::raw_data<hash, value>::output_type,
                           hash, max_t, hash, sum_v>
      f(&rd, aggregator);

  hpda::output::memory_output<hash, sum_v> mo(&f);

  engine.run();
  auto s = mo.values().size();
  EXPECT_EQ(s, 2);
  EXPECT_EQ(mo.values()[0].get<sum_v>(), 999);
  EXPECT_EQ(mo.values()[1].get<sum_v>(), 9);
}

TEST(groupby, min) {
  hpda::extractor::raw_data<hash, value> rd;
  hpda::engine engine;
  rd.set_engine(&engine);
  int sum = 0;
  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(100), i);
    rd.add_data(d);
    sum += i;
  }
  int sum2 = 0;
  for (int i = 0; i < 10; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(10), i);
    rd.add_data(d);
    sum2 += i;
  }

  typedef hpda::processor::group::min<value, sum_v> min_t;
  min_t aggregator;
  hpda::processor::groupby<hpda::extractor::raw_data<hash, value>::output_type,
                           hash, min_t, hash, sum_v>
      f(&rd, aggregator);

  hpda::output::memory_output<hash, sum_v> mo(&f);

  engine.run();
  auto s = mo.values().size();
  EXPECT_EQ(s, 2);
  EXPECT_EQ(mo.values()[0].get<sum_v>(), 0);
  EXPECT_EQ(mo.values()[1].get<sum_v>(), 0);
}

TEST(groupby, count) {
  hpda::extractor::raw_data<hash, value> rd;
  hpda::engine engine;
  rd.set_engine(&engine);
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(100), i);
    rd.add_data(d);
    sum += i;
  }
  int sum2 = 0;
  for (int i = 0; i < 10; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(10), i);
    rd.add_data(d);
    sum2 += i;
  }

  typedef hpda::processor::group::count<value, sum_v> count_t;
  count_t aggregator;
  hpda::processor::groupby<hpda::extractor::raw_data<hash, value>::output_type,
                           hash, count_t, hash, sum_v>
      f(&rd, aggregator);

  hpda::output::memory_output<hash, sum_v> mo(&f);

  engine.run();
  auto s = mo.values().size();
  EXPECT_EQ(s, 2);
  EXPECT_EQ(mo.values()[0].get<sum_v>(), 10);
  EXPECT_EQ(mo.values()[1].get<sum_v>(), 10);
}
