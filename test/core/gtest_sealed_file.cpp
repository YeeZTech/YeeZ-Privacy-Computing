#include "gtest_common.h"
#include "stbox/eth/eth_hash.h"
#include "ypc/sealed_file.h"
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>

TEST(test_sealed_file, simple) {
  ypc::simple_sealed_file::blockfile_t f;
  stbox::bytes data_hash = test_m_data(f, "tsf1", 150, 1024);

  ypc::simple_sealed_file ssf("tsf1", true);

  stbox::bytes kdata_hash = stbox::eth::keccak256_hash(ypc::bytes("test"));

  ypc::memref r;
  while (ssf.next_item(r)) {
    ypc::bytes item(r.data(), r.size());
    kdata_hash = stbox::eth::keccak256_hash(kdata_hash + item);
    r.dealloc();
  }
  EXPECT_TRUE(kdata_hash == data_hash);
}

TEST(test_sealed_file, opt) {
  ypc::simple_sealed_file::blockfile_t f;
  stbox::bytes data_hash = test_m_data(f, "tsf1", 800000, 5);

  std::cout << "done create file" << std::endl;
  ypc::sealed_file_with_cache_opt ssf("tsf1", true);

  stbox::bytes kdata_hash = stbox::eth::keccak256_hash(ypc::bytes("test"));

  ypc::memref r;
  int i = 0;
  while (ssf.next_item(r)) {
    ypc::bytes item(r.data(), r.size());
    // std::cout << i << ": " << item << std::endl;
    kdata_hash = stbox::eth::keccak256_hash(kdata_hash + item);
    r.dealloc();
    i++;
  }
  EXPECT_TRUE(kdata_hash == data_hash);
}
