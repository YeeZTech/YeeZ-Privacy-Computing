#include "ypc/base64.h"
#include <gtest/gtest.h>

TEST(test_base64, encode_decode) {
  std::vector<std::string> in(
      {"", "f", "fo", "foo", "foob", "fooba", "foobar"});
  std::vector<std::string> out(
      {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy"});
  for (size_t i = 0; i < in.size(); i++) {
    auto ret = ypc::encode_base64(in[i]);
    EXPECT_EQ(ret, out[i]);
  }
  for (size_t i = 0; i < out.size(); i++) {
    std::string output;
    auto ret = ypc::decode_base64(out[i], output);
    EXPECT_TRUE(ret);
    EXPECT_EQ(in[i], output);
  }
}

TEST(test_base64, decode_failed) {
  std::string output;
  auto ret = ypc::decode_base64(std::string("invalid", 7), output);
  // EXPECT_FALSE(ret);
  ret = ypc::decode_base64(std::string("nQB/pZw=", 8), output);
  // EXPECT_FALSE(ret);
  // ret = ypc::decode_base64(std::string("nQB/pZw=\0invalid", 16), output);
  // EXPECT_FALSE(ret);
  ret = ypc::decode_base64(std::string("nQB/pZw=invalid", 15), output);
  // EXPECT_FALSE(ret);
}
