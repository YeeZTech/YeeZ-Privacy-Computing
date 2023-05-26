#if defined(_WIN32)
/*
 * The defined WIN32_NO_STATUS macro disables return code definitions in
 * windows.h, which avoids "macro redefinition" MSVC warnings in ntstatus.h.
 */
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <bcrypt.h>
#include <ntstatus.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/random.h>
#elif defined(__OpenBSD__)
#include <unistd.h>
#else
#error "Couldn't identify the OS"
#endif

#include <limits.h>
#include <stddef.h>

#include "ypc/core/group.h"
#include "ypc/core/kgt.h"
#include <assert.h>
#include <gtest/gtest.h>

/* Returns 1 on success, and 0 on failure. */
int fill_random(unsigned char *data, size_t size) {
#if defined(_WIN32)
  NTSTATUS res =
      BCryptGenRandom(NULL, data, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  if (res != STATUS_SUCCESS || size > ULONG_MAX) {
    return 0;
  } else {
    return 1;
  }
#elif defined(__linux__) || defined(__FreeBSD__)
  /* If `getrandom(2)` is not available you should fallback to /dev/urandom */
  ssize_t res = getrandom(data, size, 0);
  if (res < 0 || (size_t)res != size) {
    return 0;
  } else {
    return 1;
  }
#elif defined(__APPLE__) || defined(__OpenBSD__)
  /* If `getentropy(2)` is not available you should fallback to either
   * `SecRandomCopyBytes` or /dev/urandom */
  int res = getentropy(data, size);
  if (res == 0) {
    return 1;
  } else {
    return 0;
  }
#endif
  return 0;
}

TEST(test_secp256k1pub_kgt, create_kgt_no_child) {
  unsigned char skey1[32];
  secp256k1_pubkey pkey1;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  EXPECT_EQ(fill_random(skey1, sizeof(skey1)), 1);
  return_val = secp256k1_ec_pubkey_create(ctx, &pkey1, skey1);
  key_node<secp256k1_pkey_group> pkey1_node(pkey1);
  std::vector<std::shared_ptr<key_node<secp256k1_pkey_group>>> children;
  kgt<secp256k1_pkey_group> k(
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey1_node), children);
  std::string res = k.to_string();
  kgt<secp256k1_pkey_group> k2(res);
  std::string res2 = k2.to_string();
  EXPECT_EQ(res, res2);
}

TEST(test_secp256k1pub_kgt, create_kgt_using_node) {
  unsigned char skey2[32], skey3[32], skey4[32];
  secp256k1_pubkey pkey1, pkey2, pkey3, pkey4;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  EXPECT_EQ(fill_random(skey2, sizeof(skey2)), 1);
  EXPECT_EQ(fill_random(skey3, sizeof(skey3)), 1);
  EXPECT_EQ(fill_random(skey4, sizeof(skey4)), 1);

  return_val = secp256k1_ec_pubkey_create(ctx, &pkey2, skey2);
  return_val = secp256k1_ec_pubkey_create(ctx, &pkey3, skey3);
  return_val = secp256k1_ec_pubkey_create(ctx, &pkey4, skey4);

  key_node<secp256k1_pkey_group> pkey1_node(pkey1);
  key_node<secp256k1_pkey_group> pkey2_node(pkey2);
  key_node<secp256k1_pkey_group> pkey3_node(pkey3);
  key_node<secp256k1_pkey_group> pkey4_node(pkey4);
  std::vector<std::shared_ptr<key_node<secp256k1_pkey_group>>> children = {
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey2_node),
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey3_node)};
  kgt<secp256k1_pkey_group> k1(
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey1_node), children);

  children.clear();
  children.push_back(k1.root());
  kgt<secp256k1_pkey_group> k4(
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey4_node), children);
  std::string res = k4.to_string();
  kgt<secp256k1_pkey_group> k2(res);
  std::string res2 = k2.to_string();
  EXPECT_EQ(res, res2);
}

TEST(test_secp256k1pub_kgt, calculate_sum_no_child) {
  unsigned char skey1[32];
  secp256k1_pubkey pkey1;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  return_val = fill_random(skey1, sizeof(skey1));
  EXPECT_EQ(return_val, 1);

  return_val = secp256k1_ec_pubkey_create(ctx, &pkey1, skey1);
  key_node<secp256k1_pkey_group> pkey1_node(pkey1);
  std::vector<std::shared_ptr<key_node<secp256k1_pkey_group>>> children;
  kgt<secp256k1_pkey_group> k(
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey1_node), children);
  return_val = k.calculate_kgt_sum();
  EXPECT_TRUE(memcmp(&pkey1, &k.sum(), sizeof(secp256k1_pubkey)) == 0);
}

TEST(test_secp256k1pub_kgt, calculate_sum_one_child) {
  unsigned char skey2[32];
  secp256k1_pubkey pkey1;
  secp256k1_pubkey pkey2;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  return_val = fill_random(skey2, sizeof(skey2));
  return_val = secp256k1_ec_pubkey_create(ctx, &pkey2, skey2);

  key_node<secp256k1_pkey_group> pkey1_node(pkey1);
  key_node<secp256k1_pkey_group> pkey2_node(pkey2);
  std::vector<std::shared_ptr<key_node<secp256k1_pkey_group>>> children = {
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey2_node)};
  kgt<secp256k1_pkey_group> k(
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey1_node), children);
  return_val = k.calculate_kgt_sum();
  secp256k1_pubkey res;
  EXPECT_TRUE(memcmp(&k.sum(), &pkey2, sizeof(secp256k1_pubkey)) == 0);
}

TEST(test_secp256k1pub_kgt, calculate_sum_multi_children) {
  unsigned char skey2[32], skey3[32];
  secp256k1_pubkey pkey1;
  secp256k1_pubkey pkey2;
  secp256k1_pubkey pkey3;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  return_val = fill_random(skey2, sizeof(skey2));
  EXPECT_EQ(return_val, 1);
  return_val = fill_random(skey3, sizeof(skey3));
  EXPECT_EQ(return_val, 1);

  return_val = secp256k1_ec_pubkey_create(ctx, &pkey2, skey2);
  return_val = secp256k1_ec_pubkey_create(ctx, &pkey3, skey3);

  key_node<secp256k1_pkey_group> pkey1_node(pkey1);
  key_node<secp256k1_pkey_group> pkey2_node(pkey2);
  key_node<secp256k1_pkey_group> pkey3_node(pkey3);
  std::vector<std::shared_ptr<key_node<secp256k1_pkey_group>>> children = {
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey2_node),
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey3_node)};
  kgt<secp256k1_pkey_group> k(
      std::make_shared<key_node<secp256k1_pkey_group>>(pkey1_node), children);
  return_val = k.calculate_kgt_sum();
  secp256k1_pubkey res;
  return_val = secp256k1_pkey_group::add(res, pkey2, pkey3);
  EXPECT_TRUE(memcmp(&res, &k.sum(), sizeof(secp256k1_pubkey)) == 0);
}

TEST(test_secp256k1pub_kgt, complex_kgt) {
  using skey_t = std::array<unsigned char, 32>;
  std::vector<skey_t> skey_vec;
  skey_t tmp_skey;
  std::vector<secp256k1_pubkey> pkey_vec;
  std::vector<key_node<secp256k1_pkey_group>> key_nodes;
  secp256k1_pubkey tmp_pkey;
  int return_val;
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(fill_random(&tmp_skey[0], 32), 1);
    skey_vec.emplace_back(tmp_skey);
    return_val = secp256k1_ec_pubkey_create(ctx, &tmp_pkey, skey_vec[i].data());
    pkey_vec.emplace_back(tmp_pkey);
    key_nodes.emplace_back(pkey_vec[i]);
  }

  std::vector<std::shared_ptr<key_node<secp256k1_pkey_group>>> children1 = {
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[0]),
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[1]),
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[2])};
  auto sub_root1 = std::make_shared<key_node<secp256k1_pkey_group>>();
  kgt<secp256k1_pkey_group> k1(sub_root1, children1);
  return_val = k1.calculate_kgt_sum();

  std::vector<std::shared_ptr<key_node<secp256k1_pkey_group>>> children2 = {
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[3]),
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[4]),
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[5])};
  auto sub_root2 = std::make_shared<key_node<secp256k1_pkey_group>>();
  kgt<secp256k1_pkey_group> k2(sub_root2, children2);
  return_val = k2.calculate_kgt_sum();

  children1.clear();
  children1.push_back(k1.root());
  children1.push_back(
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[6]));
  children1.push_back(
      std::make_shared<key_node<secp256k1_pkey_group>>(key_nodes[7]));
  children1.push_back(k2.root());
  auto root = std::make_shared<key_node<secp256k1_pkey_group>>();
  kgt<secp256k1_pkey_group> k3(root, children1);
  return_val = k3.calculate_kgt_sum();

  secp256k1_pubkey root_sum;
  return_val = secp256k1_pkey_group::add(root_sum, k1.sum(), pkey_vec[6]);
  return_val = secp256k1_pkey_group::add(root_sum, root_sum, pkey_vec[7]);
  return_val = secp256k1_pkey_group::add(root_sum, root_sum, k2.sum());
  EXPECT_TRUE(memcmp(&root_sum, &k3.sum(), sizeof(secp256k1_pubkey)) == 0);

  std::string res = k3.to_string();
  kgt<secp256k1_pkey_group> k4(res);
  std::string res2 = k4.to_string();
  EXPECT_EQ(res, res2);
}
