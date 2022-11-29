#include <glog/logging.h>
#include <gtest/gtest.h>
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <string>

define_nt(hash, std::string);
define_nt(value, uint64_t);
typedef hpda::ntobject<hash, value> data_item_t;

TEST(input_output, empty) {
  hpda::extractor::raw_data<hash, value> rd;
  hpda::engine engine;
  rd.set_engine(&engine);

  hpda::output::memory_output<hash, value> mo(&rd);

  engine.run();
  auto s = mo.values().size();
  EXPECT_EQ(s, 0);
}

TEST(input_output, basic) {
  hpda::extractor::raw_data<hash, value> rd;
  hpda::engine engine;
  rd.set_engine(&engine);

  for (int i = 0; i < 1000; i++) {
    data_item_t d;
    d.set<hash, value>(std::to_string(i), i);
    rd.add_data(d);
  }

  hpda::output::memory_output<hash, value> mo(&rd);

  engine.run();

  auto vs = mo.values();
  EXPECT_EQ(vs.size(), 1000);
  auto first = vs[0];
  auto last = vs[mo.values().size() - 1];
  // LOG(INFO) << "last : " << (mo.values()[mo.values().size() -
  // 1]).get<hash>();
  LOG(INFO) << "first: " << first.get<hash>();
  LOG(INFO) << "last : " << last.get<hash>();
  auto s = mo.values().size();
  EXPECT_EQ(s, 1000);
}
