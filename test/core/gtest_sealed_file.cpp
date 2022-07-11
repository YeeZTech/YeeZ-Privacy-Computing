#include "gtest_common.h"
#include "ypc/core/sealed_file.h"
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>

TEST(test_sealed_file, simple) {
  ypc::simple_sealed_file::blockfile_t f;
  ypc::bytes data_hash = test_m_data(f, "tsf1", 150, 1024);

  ypc::simple_sealed_file ssf("tsf1", true);

  ypc::bytes kdata_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(ypc::bytes("test"), kdata_hash);

  ypc::memref r;
  while (ssf.next_item(r)) {
    ypc::bytes item(r.data(), r.size());
    ypc::crypto::eth_sgx_crypto::hash_256(kdata_hash + item, kdata_hash);
    r.dealloc();
  }
  EXPECT_TRUE(kdata_hash == data_hash);
}

TEST(test_sealed_file, opt) {
  ypc::simple_sealed_file::blockfile_t f;
  ypc::bytes data_hash = test_m_data(f, "tsf1", 800000, 5);

  std::cout << "done create file" << std::endl;
  ypc::sealed_file_with_cache_opt ssf("tsf1", true);

  ypc::bytes kdata_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(ypc::bytes("test"), kdata_hash);

  ypc::memref r;
  int i = 0;
  while (ssf.next_item(r)) {
    ypc::bytes item(r.data(), r.size());
    // std::cout << i << ": " << item << std::endl;
    ypc::crypto::eth_sgx_crypto::hash_256(kdata_hash + item, kdata_hash);
    r.dealloc();
    i++;
  }
  EXPECT_TRUE(kdata_hash == data_hash);
}
