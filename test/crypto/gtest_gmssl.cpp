#include "corecommon/crypto/gmssl/sm2_ecc.h"
#include "corecommon/crypto/gmssl/sm3_hash.h"
#include "corecommon/crypto/gmssl/sm4_aes.h"
#include "stbox/stx_status.h"
#include "ypc/byte.h"
#include <gmssl/sm2.h>
#include <gmssl/sm4.h>
#include <gtest/gtest.h>

TEST(test_sm3_hash, sha3_256) {
  std::string msg = "hello";
  uint8_t hash[32];
  uint32_t ret = ypc::crypto::sm3_hash::sha3_256((const uint8_t *)&msg[0],
                                                 msg.size(), hash);
  EXPECT_EQ(ret, 0);
  ypc::bytes expect_hash =
      ypc::hex_bytes(
          "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824")
          .as<ypc::bytes>();
  EXPECT_TRUE(memcmp(hash, expect_hash.data(), 32) == 0);
}

TEST(test_sm3_hash, get_msg_hash_size) {
  EXPECT_EQ(ypc::crypto::sm3_hash::get_msg_hash_size(), 32);
}


TEST(test_sm3_hash, msg_hash) {
  std::string raw_msg = "hello";
  uint8_t hash[32];
  uint32_t ret = ypc::crypto::sm3_hash::msg_hash((const uint8_t *)&raw_msg[0],
                                                 raw_msg.size(), hash, 32);
  EXPECT_EQ(ret, 0);
  ypc::bytes expect_hash =
      ypc::hex_bytes(
          "becbbfaae6548b8bf0cfcad5a27183cd1be6093b1cceccc303d9c61d0a645268")
          .as<ypc::bytes>();
  EXPECT_TRUE(memcmp(hash, expect_hash.data(), 32) == 0);
}

TEST(test_sm2_ecc, get_private_key_size) {
  EXPECT_EQ(ypc::crypto::sm2_ecc::get_private_key_size(), 32);
}

TEST(test_sm2_ecc, get_public_key_size) {
  EXPECT_EQ(ypc::crypto::sm2_ecc::get_public_key_size(), 64);
}

TEST(test_sm2_ecc, gen_private_key) {
  uint8_t skey[32];
  uint32_t ret = ypc::crypto::sm2_ecc::gen_private_key(32, skey);
  EXPECT_EQ(ret, 0);
}

void get_expected_pkey(const ypc::bytes &skey, ypc::bytes &pkey) {
  SM2_KEY tmp;
  sm2_key_set_private_key(&tmp, skey.data());
  memcpy(pkey.data(), &tmp, 64);
  /*
  for (int i = 0; i < 64; i++) {
    printf("%02x", *(pkey.data() + i));
  }
  std::cout << std::endl;
  */
}

void show_hex(uint8_t *data, size_t size) {
  for (int i = 0; i < size; i++) {
    printf("%02x", *(data + i));
  }
  std::cout << std::endl;
}

TEST(test_sm2_ecc, generate_pkey_from_skey) {
  ypc::hex_bytes skey_hex(
      "4f16ab84f1d146f036332f30cc00d76c6b598c01887d88d935e728d221f4506e");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();
  uint8_t pkey[64];
  uint32_t ret = ypc::crypto::sm2_ecc::generate_pkey_from_skey(
      skey.data(), skey.size(), pkey, 64);
  EXPECT_EQ(ret, 0);
  ypc::bytes expect_pkey(64);
  get_expected_pkey(skey, expect_pkey);
  EXPECT_TRUE(memcmp(pkey, expect_pkey.data(), 64) == 0);
}

TEST(test_sm2_ecc, get_signature_size) {
  EXPECT_EQ(ypc::crypto::sm2_ecc::get_signature_size(), 64);
}

TEST(test_sm2_ecc, sign) {
  ypc::hex_bytes skey_hex(
      "4f16ab84f1d146f036332f30cc00d76c6b598c01887d88d935e728d221f4506e");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();
  std::string data = "hello";
  uint8_t hash[32];
  uint32_t ret = ypc::crypto::sm3_hash::msg_hash((const uint8_t *)&data[0],
                                                 data.size(), hash, 32);

  uint8_t sig[64];
  ret = ypc::crypto::sm2_ecc::sign_message(skey.data(), skey.size(), hash, 32,
                                           sig, 64);
  ypc::bytes expect_pkey(64);
  get_expected_pkey(skey, expect_pkey);
  ret = ypc::crypto::sm2_ecc::verify_signature(hash, 32, sig, 64,
                                               expect_pkey.data(), 64);

  EXPECT_EQ(ret, 0);

   ypc::bytes false_sig =
      ypc::hex_bytes(
          "bcebe54da5082467e5946b9c7bce8ca64bb3025574a0e0ed4eaeec5a7099d81b9ae496e0d4d8ef1b03d2ce5abc5a64806fad321c44a3f987e899491c6782d786")
          .as<ypc::bytes>();
  //bcebe54da5082467e5946b9c7bce8ca64bb3025574a0e0ed4eaeec5a7099d81b9ae496e0d4d8ef1b03d2ce5abc5a64806fad321c44a3f987e899491c6782d787

  uint32_t fail_res = ypc::crypto::sm2_ecc::verify_signature(hash, 32, false_sig.data(), 64,
                                               expect_pkey.data(), 64);

  EXPECT_EQ(fail_res, stbox::stx_status::sm2_get_false_sign);
}


TEST(test_sm2_ecc, ecdh_shared_key) {
  ypc::hex_bytes skey_hex_a(
      "4f16ab84f1d146f036332f30cc00d76c6b598c01887d88d935e728d221f4506e");
  ypc::bytes skey_a = skey_hex_a.as<ypc::bytes>();
  ypc::bytes expect_pkey_a(64);
  get_expected_pkey(skey_a, expect_pkey_a);
  uint8_t shared_key_a[16], shared_key_b[16];

  ypc::hex_bytes skey_hex_b(
      "70c39e6e85f850193a623178b8bd4c798a36e62e71bb5ca106768246c2cb8baf");
  ypc::bytes skey_b = skey_hex_b.as<ypc::bytes>();
  ypc::bytes expect_pkey_b(64);
  get_expected_pkey(skey_b, expect_pkey_b);

  uint32_t ret = ypc::crypto::sm2_ecc::ecdh_shared_key(skey_a.data(), 32,
                                                expect_pkey_b.data(), 64,
                                                shared_key_a, 16);

  ret = ypc::crypto::sm2_ecc::ecdh_shared_key(skey_b.data(), 32,
                                                expect_pkey_a.data(), 64,
                                                shared_key_b, 16);
  EXPECT_TRUE(memcmp(shared_key_a, shared_key_b, 16) == 0);
  EXPECT_EQ(ret, 0);
}

TEST(test_sm4_aes, get_cipher_size) {
  std::string data = "hello";
  uint32_t data_size = data.size();
  EXPECT_EQ(ypc::crypto::sm4_aes::get_cipher_size(data_size), data_size + 12);
}

TEST(test_sm4_aes, encrypt_and_decrypt_with_prefix) {
  ypc::bytes key("k3Men*p/2.3j4abB");
  std::string data = "this|is|a|test|message";
  uint32_t data_size = data.size();
  uint32_t prefix = 0x1;
  uint32_t cipher_size = ypc::crypto::sm4_aes::get_cipher_size(data_size);
  uint8_t cipher[cipher_size] = {0};
  uint8_t out_mac[16] = {0};

  uint32_t ret = ypc::crypto::sm4_aes::encrypt_with_prefix(
      key.data(), key.size(), (const uint8_t *)&data[0], data_size, prefix,
      cipher, cipher_size, out_mac);
  EXPECT_EQ(ret, 0);

  std::string decrypted_data(data.size(), '0');
  ret = ypc::crypto::sm4_aes::decrypt_with_prefix(
      key.data(), key.size(), cipher, cipher_size, prefix,
      (uint8_t *)&decrypted_data[0], data_size, out_mac);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(data, decrypted_data);
}

TEST(test_sm4_aes, gcm_encrypt_decrypt) {
  ypc::bytes skey(32);
  ypc::crypto::sm2_ecc::gen_private_key(skey.size(), skey.data());
  ypc::bytes key(skey.data(), 16);

  std::string data = "this|is|a|test|message";
  std::string msg(data.size(), '0');
  // uint32_t cipher_size = ypc::crypto::sm4_aes::get_cipher_size(data.size());
  // std::cout << "data size: " << data.size() << ", cipher size: " <<
  // cipher_size
  //<< std::endl;
  // uint8_t cipher[cipher_size] = {0};
  uint8_t cipher[data.size()] = {0};
  uint8_t tag[16] = {0};
  // encrypt
  SM4_KEY sm4_key;
  sm4_set_encrypt_key(&sm4_key, key.data());
  // size_t sm4_key_size = sizeof(SM4_KEY);
  // uint8_t encrypt_key[sm4_key_size];
  // memcpy(encrypt_key, &sm4_key, sm4_key_size);
  // show_hex(encrypt_key, sm4_key_size);
  char aad[64] = {0};
  // uint8_t *p_iv_text = cipher + data.size();
  uint8_t p_iv_text[12] = {0};
  int ret =
      sm4_gcm_encrypt(&sm4_key, p_iv_text, 12, (const uint8_t *)aad, 64,
                      (const uint8_t *)&data[0], data.size(), cipher, 16, tag);
  std::cout << "sm4_gcm_encrypt return: " << ret << std::endl;

  // decrypt
  sm4_set_encrypt_key(&sm4_key, key.data());
  // uint8_t decrypt_key[sm4_key_size];
  // memcpy(decrypt_key, &sm4_key, sm4_key_size);
  // show_hex(decrypt_key, sm4_key_size);
  ret = sm4_gcm_decrypt(&sm4_key, p_iv_text, 12, (const uint8_t *)aad, 64,
                        cipher, data.size(), tag, 16, (uint8_t *)&msg[0]);
  std::cout << "sm4_gcm_decrypt return: " << ret << std::endl;
  std::cout << "msg: " << msg << std::endl;
}

TEST(test_sm4_aes, get_data_size) {
  ypc::bytes cipher =
      ypc::hex_bytes(
          "943fac246391f0653d32a92c9820e36c3e66d6672a83e2a93d18451caae0b2c1dba8")
          .as<ypc::bytes>();
  uint32_t cipher_size = cipher.size();
  EXPECT_EQ(ypc::crypto::sm4_aes::get_data_size(cipher_size), cipher_size - 12);
}
