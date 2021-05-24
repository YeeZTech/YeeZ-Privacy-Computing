#include "ypc/byte.h"
#include "ypc/sha.h"
#include <gtest/gtest.h>

TEST(test_sha, encode) {
  // clang-format off
  std::vector<std::pair<std::string,std::string>> v({
      std::make_pair("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"),
      std::make_pair("\n", "01ba4719c80b6fe911b091a7c05124b64eeece964e09c058ef8f9805daca546b"),
      std::make_pair("hello", "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824"),
      std::make_pair("ypc", "9ed27916fabb02183eb305787a60edfd589911edd4863a692e5b27b8c3c16c81"),
      std::make_pair("sgx", "86523bff7e894750d2ab1d28c6034d1d4a8a4b119e03840b86a0149fd2a7b1dd")
  });
  // clang-format on

  for (auto &ele : v) {
    auto ret = ypc::SHA256(ele.first.c_str(), ele.first.size());
    auto bytes = ypc::hex_bytes(ele.second.c_str()).as<ypc::bytes>();
    EXPECT_TRUE(bytes == ret);
  }
}
