#include "gtest_common.h"
#include "stbox/eth/eth_hash.h"
#include "ypc/blockfile.h"
#include <functional>
#include <gtest/gtest.h>
#include <random>

// typedef ypc::blockfile<0x4788d13e7fefe21f, 1024 * 1024,
// 256 * ypc::max_item_size>
// bft;
typedef ypc::blockfile<0x29384792, 16, 1024> bft;

TEST(test_blockfile, simple) { ypc::blockfile<0x29384792, 16, 1024> t; }

void test_1_data(const ypc::bytes &k) {
  bft f;
  f.open_for_write("tf1");

  f.append_item(k.data(), k.size());
  f.close();
  f.open_for_read("tf1");
  ypc::memref r;
  bool t = f.next_item(r);
  EXPECT_EQ(t, true);
  EXPECT_EQ(r.size(), k.size());
  ypc::bytes k_prime(r.data(), r.size());
  EXPECT_TRUE(k == k_prime);

  t = f.next_item(r);
  EXPECT_EQ(t, false);
  f.close();
}


TEST(test_blockfile, 1_data) {
  // not exceed length

  test_1_data(ypc::bytes("123456"));
  test_1_data(random_string(2048));
  // exceed length
}

TEST(test_blockfile, many_small_data) {

  bft f1;
  test_m_data(f1, "tf2", 150, 128);
  bft f2;
  test_m_data(f2, "tf3", 16, 1023);
}

TEST(test_blockfile, many_large_data) {
  bft f1;
  test_m_data(f1, "tf4", 150, 1023);
}

TEST(test_blockfile, exceed_block_num) {}
