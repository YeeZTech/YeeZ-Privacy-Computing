#include "ypc/common/crypto_prefix.h"
#include "ypc/core/byte.h"
#include "ypc/terminus/crypto_pack.h"
#include <gtest/gtest.h>

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
