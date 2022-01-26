#include "common/crypto_prefix.h"
#include "ypc/byte.h"
#include "ypc/terminus/crypto_pack.h"
#include <gtest/gtest.h>

TEST(test_terminus_intel_sgx, decrypt) {
  auto cp = ypc::terminus::intel_sgx_and_eth_compatible();
  ypc::bytes skey =
      ypc::hex_bytes(
          "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043")
          .as<ypc::bytes>();
  ypc::bytes pkey =
      ypc::hex_bytes(
          "a042a0ea6d9c0f1ec0f8563c0292a35635f538ba3bd1bc690e282eeaa0a3c744c3e7"
          "e2e25228881222ce014b7ac087d797b8e4482e9b4e243f5db0ad1e0fc266")
          .as<ypc::bytes>();
  ypc::bytes content =
      ypc::hex_bytes(
          "2012f62ed7c7c71dfdf7997accf6e3924a2d0cab9d6b15fd04a5d80f83ed15902836"
          "918299893bbc871db0299dad84ba3078ad24374409a2e82f750b8fe57983f6013ca6"
          "a1087c51f7f2314022e3523ae05ea5c2171f699b6d661404")
          .as<ypc::bytes>();

  ypc::bytes ret =
      cp->ecc_decrypt(content, skey, ypc::utc::crypto_prefix_arbitrary);
  EXPECT_TRUE(ret.size() > 0);
  ypc::bytes expect("hello world!");
  LOG(INFO) << "ret: " << std::string((const char *)ret.data(), ret.size());
  EXPECT_EQ(ret == expect, true);
}

TEST(test_terminus_intel_sgx, encrypt_decrypt) {
  auto cp = ypc::terminus::intel_sgx_and_eth_compatible();
  ypc::bytes skey =
      ypc::hex_bytes(
          "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043")
          .as<ypc::bytes>();
  ypc::bytes pkey = cp->gen_ecc_public_key_from_private_key(skey);
  ypc::bytes content("hello world!");
  ypc::bytes ret =
      cp->ecc_encrypt(content, pkey, ypc::utc::crypto_prefix_arbitrary);
  EXPECT_TRUE(ret.size() > 0);

  LOG(INFO) << "ret is: " << ret;
  ypc::bytes expect =
      cp->ecc_decrypt(ret, skey, ypc::utc::crypto_prefix_arbitrary);
  EXPECT_TRUE(expect.size() > 0);
  EXPECT_TRUE(content == expect);
}

TEST(test_terminus_intel_sgx, sign) {
  auto cp = ypc::terminus::intel_sgx_and_eth_compatible();
  ypc::bytes skey =
      ypc::hex_bytes(
          "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043")
          .as<ypc::bytes>();

  ypc::bytes pkey = cp->gen_ecc_public_key_from_private_key(skey);

  ypc::bytes content("hello world!");
  ypc::bytes sig =
      ypc::hex_bytes(
          "cd7c17520238fa21586d8ea5a35e6f8d7c8c4fc9980ff37424b42ce2d89fb138414a"
          "9ee1250cb1d32d3be0469491d555e136563bcdfbfa9bcea401cb798b4aee1b")
          .as<ypc::bytes>();
  bool b = cp->verify_message_signature(sig, content, pkey);
  EXPECT_TRUE(b);
}

TEST(test_terminus_intel_sgx, sign_and_verify) {

  auto cp = ypc::terminus::intel_sgx_and_eth_compatible();
  ypc::bytes skey =
      ypc::hex_bytes(
          "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043")
          .as<ypc::bytes>();

  ypc::bytes pkey = cp->gen_ecc_public_key_from_private_key(skey);

  ypc::bytes content("hello world!");
  ypc::bytes sig = cp->sign_message(content, skey);
  LOG(INFO) << "sig: " << sig;
  EXPECT_TRUE(sig.size() > 0);
  bool b = cp->verify_message_signature(sig, content, pkey);
  EXPECT_TRUE(b);
}
