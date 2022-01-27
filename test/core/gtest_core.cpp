#include "test_ypc_module.h"
#include "ypc/byte.h"
#include <gtest/gtest.h>
#include <memory>

class TestCoreInstance : public testing::Test {
protected:
  void SetUp() override {
    if (!mod) {
      mod = std::make_shared<test_ypc_sgx_module>(
          "../lib/test_ypc_enclave.signed.so");
    }
  }
  std::shared_ptr<test_ypc_sgx_module> mod;
};

TEST_F(TestCoreInstance, simple) {
  ypc::hex_bytes skey_hex(
      "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();
  ypc::bytes sealed_skey;
  auto ret = mod->seal_data(skey, sealed_skey);
  EXPECT_EQ(ret, 0);
  std::cout << "simple raw data size: " << skey.size() << std::endl;
  std::cout << "simple sealed data size: " << sealed_skey.size() << std::endl;
}

TEST_F(TestCoreInstance, medium) {
  ypc::bytes skey(4096);
  ypc::bytes sealed_skey;
  auto ret = mod->seal_data(skey, sealed_skey);
  EXPECT_EQ(ret, 0);
  std::cout << "medium raw data size: " << skey.size() << std::endl;
  std::cout << "medium sealed data size: " << sealed_skey.size() << std::endl;
}

TEST_F(TestCoreInstance, large) {
  ypc::bytes skey(4096 * 32);
  ypc::bytes sealed_skey;
  auto ret = mod->seal_data(skey, sealed_skey);
  EXPECT_EQ(ret, 0);
  std::cout << "large raw data size: " << skey.size() << std::endl;
  std::cout << "large sealed data size: " << sealed_skey.size() << std::endl;
}
