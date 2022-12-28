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

  // ypc::memref r;
  std::unique_ptr<char[]> buf(new char[1024]);
  size_t buf_size;
  while (ssf.next_item(buf.get(), 1024, buf_size) ==
         ypc::simple_sealed_file::blockfile_t::succ) {
    ypc::bytes item(buf.get(), buf_size);
    ypc::crypto::eth_sgx_crypto::hash_256(kdata_hash + item, kdata_hash);
  }
  EXPECT_TRUE(kdata_hash == data_hash);
}

#if 0
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
#endif
