#include "./test_crypto_module.h"
#include <gtest/gtest.h>
#include <iostream>

namespace {
class crypto_test : public testing::Test {
protected:
  void SetUp() override {
    m_module.reset(
        new test_crypto_sgx_module("../lib/test_crypto_enclave.signed.so"));
  }

protected:
  std::unique_ptr<test_crypto_sgx_module> m_module;
};
} // namespace

TEST_F(crypto_test, aes_cmac_msg) {
  ypc::bytes pkey("k3Men*p/2.3j4abB");
  ypc::bytes msg("this|is|a|test|message");
  ypc::bytes result = m_module->aes_cmac_msg(pkey, msg);

  ypc::hex_bytes expect("0125c538f8be7c4eea370f992a4ffdcb");
  EXPECT_TRUE(expect.as<ypc::bytes>() == result);
}

TEST_F(crypto_test, gen_pub_key) {
  ypc::hex_bytes skey_hex(
      "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();

  ypc::hex_bytes expect_hex(
      "5d7ee992f48ffcdb077c2cb57605b602bd4029faed3e91189c7fb9fccc72771e4"
      "5b7aa166766e2ad032d0a195372f5e2d20db792901d559ab0d2bfae10ecea97");

  ypc::bytes pkey = m_module->test_generate_pkey(skey);
  EXPECT_TRUE(pkey == expect_hex.as<ypc::bytes>());
}

TEST_F(crypto_test, ecdh) {
  ypc::hex_bytes skey_hex(
      "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();

  ypc::hex_bytes hex_pkey(
      "a042a0ea6d9c0f1ec0f8563c0292a35635f538ba3bd1bc690e282eeaa0a3c744c3e7e2"
      "e25228881222ce014b7ac087d797b8e4482e9b4e243f5db0ad1e0fc266");
  ypc::bytes pkey = hex_pkey.as<ypc::bytes>();

  ypc::bytes shared_key = m_module->ecdh(pkey, skey);

  ypc::hex_bytes expect("bb6ad3431e19a07471acd2778267d8a5");
  EXPECT_TRUE(shared_key.as<ypc::hex_bytes>() == expect);
}

TEST_F(crypto_test, enrypt_decrypt) {
  ypc::bytes data("hello world");

  ypc::hex_bytes skey_hex(
      "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();

  ypc::bytes pkey = m_module->test_generate_pkey(skey);

  ypc::bytes cipher =
      m_module->encrypt(pkey, data, ypc::utc::crypto_prefix_forward);

  ypc::bytes dd =
      m_module->decrypt(skey, cipher, ypc::utc::crypto_prefix_forward);

  EXPECT_TRUE(dd == data);
}

TEST_F(crypto_test, decrypt_data) {

  ypc::hex_bytes skey_hex(
      "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();

  ypc::bytes cipher =
      ypc::hex_bytes("90c9f0b8224400f80437ee42928cf4f4d9d45f2207f27d9848d6e806"
                     "0612b5a12d71ca96c777ecbfc3b3701f228e33c0b93e3628de18e2ad0"
                     "3e2ada74b3abd477694bd6fa1580f575fc1c73cd1b5f6ec8170a24b27"
                     "3be459e3e69b9c48da325133b773310ba0cf")
          .as<ypc::bytes>();

  ypc::bytes data("hello world");

  ypc::bytes dd =
      m_module->decrypt(skey, cipher, ypc::utc::crypto_prefix_forward);
  EXPECT_TRUE(dd == data);
}

TEST_F(crypto_test, sign) {
  ypc::bytes cipher =
      ypc::hex_bytes(
          "01000000d0cf76a0cc00ca3bf8636f6044f64749927ccd8420743b0f998fbc9"
          "cbbb39c69d6358c69f1bce8be69c7ba6d2715ee7be1100780c1074f80325375"
          "4dbae564bf398edaa6cd966e10e658b76c8c5cce54533a261f7cf236a66fd6b"
          "56fd9f41bc9217501d00082b3b6a87a5a690c5802ae50feff0a362a609ab5a6"
          "eecafdb2289890bd7261871c04fb5d7323d4fc750f6444b067a12a96efbe24c"
          "62572156caa514657d4a535101d2147337f41f51fcdfcf8f43a53df0885cecb"
          "1a55559c0020a7c208b0cc4a44675facfe04624c81a62b5a28cdea")
          .as<ypc::bytes>();

  ypc::bytes sk =
      ypc::hex_bytes(
          "97dc393924f6d82795f4caf56a79b88ff4df036b04db076f2d254f6bfe0ea869")
          .as<ypc::bytes>();

  std::cout << "start sign" << std::endl;
  ypc::bytes sig = m_module->sign_message(sk, cipher);
  std::cout << "end sign" << std::endl;

  ypc::bytes data_hash =
      ypc::hex_bytes(
          "3e0d3a43f4f45ba7a1234759c2ffa4028a44599d4ab29bec532bd2057c0f9141")
          .as<ypc::bytes>();
  std::cout << "start sign" << std::endl;
  ypc::bytes data_hash_sig = m_module->sign_message(sk, data_hash);
  std::cout << "data hash sig: " << data_hash_sig << std::endl;

  ypc::bytes pkey =
      ypc::hex_bytes(
          "362a609ab5a6eecafdb2289890bd7261871c04fb5d7323d4fc750f6444b067a12a96"
          "e"
          "fbe24c62572156caa514657d4a535101d2147337f41f51fcdfcf8f43a53")
          .as<ypc::bytes>();

  std::cout << "start verify" << std::endl;
  bool v = m_module->verify_message(cipher, sig, pkey);
  std::cout << "end verify" << std::endl;
  EXPECT_EQ(v, true);
}
