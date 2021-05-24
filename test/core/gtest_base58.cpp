#include "ypc/byte.h"
#include <gtest/gtest.h>
#include <iostream>
#include <random>

// clang-format off
std::vector<std::pair<std::string,std::string>> v({
    std::make_pair("", ""),
    std::make_pair("61", "2g"),
    std::make_pair("626262", "a3gV"),
    std::make_pair("636363", "aPEr"),
    std::make_pair("73696d706c792061206c6f6e6720737472696e67", "2cFupjhnEsSn59qHXstmK2ffpLv2"),
    std::make_pair("00eb15231dfceb60925886b67d065299925915aeb172c06647", "1NS17iag9jJgTHD1VXjvLCEnZuQ3rJDE9L"),
    std::make_pair("516b6fcd0f", "ABnLTmg"),
    std::make_pair("bf4f89001e670274dd", "3SEo3LWLoPntC"),
    std::make_pair("572e4794", "3EFU7m"),
    std::make_pair("ecac89cad93923c02321", "EJDM8drfXA6uyA"),
    std::make_pair("10c8511e", "Rt5zm"),
    std::make_pair("00000000000000000000", "1111111111"),
    std::make_pair("000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd43dc62a641155a5", "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"),
    std::make_pair("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff", "1cWB5HCBdLjAuqGGReWE3R3CguuwSjw6RHn39s2yuDRTS5NsBgNiFpWgAnEx6VQi8csexkgYw3mdYrMHr8x9i7aEwP8kZ7vccXWqKDvGv3u1GxFKPuAkn8JCPPGDMf3vMMnbzm6Nh9zh1gcNsMvH3ZNLmP5fSG6DGbbi2tuwMWPthr4boWwCxf7ewSgNQeacyozhKDDQQ1qL5fQFUW52QKUZDZ5fw3KXNQJMcNTcaB723LchjeKun7MuGW5qyCBZYzA1KjofN1gYBV3NqyhQJ3Ns746GNuf9N2pQPmHz4xpnSrrfCvy6TVVz5d4PdrjeshsWQwpZsZGzvbdAdN8MKV5QsBDY")
});
// clang-format on

TEST(test_base58, encode_pointer) {
  for (auto &ele : v) {
    auto from_hex = ypc::hex_bytes(ele.first.c_str()).as<ypc::bytes>();
    auto from_base58 = ypc::base58_bytes(ele.second.c_str()).as<ypc::bytes>();
    EXPECT_TRUE(from_hex == from_base58);

    ypc::base58_bytes base58 = from_hex.as<ypc::base58_bytes>();
    EXPECT_TRUE(base58 == ypc::base58_bytes(ele.second.c_str()));
  }
}

TEST(test_base58, encode_vector) {
  for (auto &ele : v) {
    auto from_hex = ypc::hex_bytes(ele.first.c_str()).as<ypc::bytes>();
    std::vector<unsigned char> vch;
    for (size_t i = 0; i < from_hex.size(); i++) {
      vch.push_back(from_hex[i]);
    }
    auto encoded_str = encode_base58(vch);
    EXPECT_EQ(encoded_str, ele.second);
  }
}

TEST(test_base58, decode_pointer) {
  for (auto &ele : v) {
    std::vector<unsigned char> vch;
    auto ret = decode_base58(ele.second.c_str(), vch);
    EXPECT_TRUE(ret);
    auto from_hex = ypc::hex_bytes(ele.first.c_str()).as<ypc::bytes>();
    EXPECT_EQ(from_hex.size(), vch.size());
    for (size_t i = 0; i < vch.size(); i++) {
      EXPECT_EQ(vch[i], from_hex[i]);
    }
  }
}

TEST(test_base58, decode_string) {
  for (auto &ele : v) {
    std::vector<unsigned char> vch;
    auto ret = decode_base58(ele.second, vch);
    EXPECT_TRUE(ret);
    auto from_hex = ypc::hex_bytes(ele.first.c_str()).as<ypc::bytes>();
    EXPECT_EQ(from_hex.size(), vch.size());
    for (size_t i = 0; i < vch.size(); i++) {
      EXPECT_EQ(vch[i], from_hex[i]);
    }
  }
}

TEST(test_base58, decode_failed) {
  std::vector<unsigned char> vch;
  EXPECT_FALSE(decode_base58("invalid", vch));
  EXPECT_FALSE(decode_base58(std::string("invalid"), vch));
  // EXPECT_FALSE(decode_base58(std::string("\0invalid", 8), vch));
  EXPECT_TRUE(decode_base58(std::string("good", 4), vch));
  EXPECT_FALSE(decode_base58(std::string("bad0IOl", 7), vch));
  EXPECT_FALSE(decode_base58(std::string("goodbad0IOl", 11), vch));
  // EXPECT_FALSE(decode_base58(std::string("good\0bad0IOl", 12), vch));
  EXPECT_FALSE(decode_base58(" \t\n\v\f\r skip \r\f\v\n\t a", vch));
  EXPECT_TRUE(decode_base58(" \t\n\v\f\r skip \r\f\v\n\t ", vch));
}

TEST(test_base58, random_encode) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dis(0, 256);
  for (size_t i = 0; i < 1000; i++) {
    size_t len = dis(mt);
    std::vector<unsigned char> vch;
    for (size_t j = 0; j < len; j++) {
      vch.push_back(dis(mt));
    }
    auto encoded = encode_base58(vch);
    std::vector<unsigned char> ret;
    auto flag = decode_base58(encoded, ret);
    EXPECT_TRUE(flag);
    for (size_t j = 0; j < len; j++) {
      EXPECT_EQ((int)vch[j], (int)ret[j]);
    }
  }
}
