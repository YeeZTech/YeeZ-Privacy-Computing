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

#include "ypc/corecommon/crypto/group.h"
#include "ypc/core/byte.h"
#include "ypc/core/kgt_json.h"
#include <assert.h>
#include <gtest/gtest.h>
#include <iostream>

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

TEST(test_secp256k1pri_kgtjson, create_kgt_no_child) {
  ypc::crypto::secp256k1_skey_group::key_t skey1;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  EXPECT_EQ(fill_random(skey1.data, sizeof(skey1)), 1);
  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey1_node(skey1);
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> children;
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey1_node),
      children);
  stbox::bytes res_b = k.to_bytes();
  std::ostringstream oss;
  for(int i = 0; i < sizeof(skey1.data); i++){
    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(skey1.data[i]);
  }
  std::string skey1_str = oss.str();
  std::string jsonstr = "{\"value\":\"" + skey1_str + "\",\"children\": \"\"}\n";
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k2(jsonstr);
  stbox::bytes res2_b = k2.to_bytes();
  EXPECT_EQ(memcmp(res_b.data(), res2_b.data(), res_b.size()), 0);
}

TEST(test_secp256k1pri_kgtjson, create_kgt_using_node) {
  ypc::crypto::secp256k1_skey_group::key_t skey1, skey2, skey3, skey4;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  EXPECT_EQ(fill_random(skey1.data, sizeof(skey1)), 1);
  EXPECT_EQ(fill_random(skey2.data, sizeof(skey2)), 1);
  EXPECT_EQ(fill_random(skey3.data, sizeof(skey3)), 1);
  EXPECT_EQ(fill_random(skey4.data, sizeof(skey4)), 1);

  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey1_node(skey1);
  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey2_node(skey2);
  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey3_node(skey3);
  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey4_node(skey4);
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> children = {
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey2_node),
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey3_node)};
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k1(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey1_node),
      children);

  children.clear();
  children.push_back(k1.root());
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k4(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey4_node),
      children);
  std::string res = k4.to_string();
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k2(res);
  std::string res2 = k2.to_string();
  EXPECT_EQ(res, res2);
}
/*
TEST(test_secp256k1pri_kgtjson, skey_pkey_kgt_conversion) {
  ypc::crypto::secp256k1_skey_group::key_t skey1;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  EXPECT_EQ(fill_random(skey1.data, sizeof(skey1)), 1);
  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey1_node(skey1);
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> children;
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey1_node),
      children);
  std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_pkey_group>> pkey_kgt = k.gen_pkey_kgt_from_skey_kgt(k.root());
  std::unordered_map<stbox::bytes, stbox::bytes> peer;
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_pkey_group>>> pkey_kgt_leaves = pkey_kgt.leaves();
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> skey_kgt_leaves = k.leaves();
  for(int i = 0; i < pkey_kgt_leaves.size(); i++){
    stbox::bytes data_pkey_b((uint8_t *)&pkey_kgt_leaves[i]->key_val, sizeof(pkey_kgt_leaves[i]->key_val));
    stbox::bytes data_skey_b((uint8_t *)&skey_kgt_leaves[i]->key_val, sizeof(skey_kgt_leaves[i]->key_val));
    if (peer.find(data_pkey_b) != peer.end()) {
      peer.insert(std::make_pair(data_pkey_b, data_skey_b));
    }
  }
  std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>> skey_kgt_copy = k.construct_skey_kgt_with_pkey_kgt(pkey_kgt->root(), peer);
  std::string res = k.to_string();
  std::string res2 = skey_kgt_copy.to_string();
  EXPECT_EQ(res, res2);
}

TEST(test_secp256k1pri_kgtjson, calculate_sum_no_child) {
  secp256k1_skey_group::key_t skey1;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  return_val = fill_random(skey1.data, sizeof(skey1));
  EXPECT_EQ(return_val, 1);

  ypc::key_node<secp256k1_skey_group> skey1_node(skey1);
  std::vector<std::shared_ptr<ypc::key_node<secp256k1_skey_group>>> children;
  ypc::kgt_json<secp256k1_skey_group> k(
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(skey1_node),
      children);
  return_val = k.calculate_kgt_sum();
  EXPECT_TRUE(memcmp(&skey1, &k.sum(), sizeof(secp256k1_skey_group::key_t)) ==
              0);
}

TEST(test_secp256k1pri_kgtjson, calculate_sum_one_child) {
  secp256k1_skey_group::key_t skey1;
  secp256k1_skey_group::key_t skey2;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  return_val = fill_random(skey1.data, sizeof(skey1.data));
  EXPECT_EQ(return_val, 1);
  return_val = fill_random(skey2.data, sizeof(skey2.data));
  EXPECT_EQ(return_val, 1);

  ypc::key_node<secp256k1_skey_group> skey1_node(skey1);
  ypc::key_node<secp256k1_skey_group> skey2_node(skey2);
  std::vector<std::shared_ptr<ypc::key_node<secp256k1_skey_group>>> children = {
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(skey2_node)};
  ypc::kgt_json<secp256k1_skey_group> k(
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(skey1_node),
      children);
  return_val = k.calculate_kgt_sum();
  EXPECT_TRUE(memcmp(&skey2, &k.sum(), sizeof(secp256k1_skey_group)) == 0);
}

TEST(test_secp256k1pri_kgt_json, calculate_sum_multi_children) {
  secp256k1_skey_group::key_t skey1;
  secp256k1_skey_group::key_t skey2;
  secp256k1_skey_group::key_t skey3;
  int return_val;

  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  return_val = fill_random(skey1.data, sizeof(skey1.data));
  EXPECT_EQ(return_val, 1);
  return_val = fill_random(skey2.data, sizeof(skey2.data));
  EXPECT_EQ(return_val, 1);
  return_val = fill_random(skey3.data, sizeof(skey3.data));
  EXPECT_EQ(return_val, 1);

  ypc::key_node<secp256k1_skey_group> skey1_node(skey1);
  ypc::key_node<secp256k1_skey_group> skey2_node(skey2);
  ypc::key_node<secp256k1_skey_group> skey3_node(skey3);
  std::vector<std::shared_ptr<ypc::key_node<secp256k1_skey_group>>> children = {
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(skey2_node),
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(skey3_node)};
  ypc::kgt_json<secp256k1_skey_group> k(
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(skey1_node),
      children);
  return_val = k.calculate_kgt_sum();
  secp256k1_skey_group::key_t res;
  return_val = secp256k1_skey_group::add(res, skey2, skey3);
  EXPECT_TRUE(memcmp(&res, &k.sum(), sizeof(secp256k1_skey_group::key_t)) == 0);
}

TEST(test_secp256k1pri_kgtjson, complex_kgt) {
  using skey_t = secp256k1_skey_group::key_t;
  std::vector<skey_t> skey_vec;
  skey_t tmp_skey;
  std::vector<ypc::key_node<secp256k1_skey_group>> key_nodes;
  int return_val;
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(fill_random(&tmp_skey.data[0], 32), 1);
    skey_vec.emplace_back(tmp_skey);
    key_nodes.emplace_back(skey_vec[i]);
  }

  std::vector<std::shared_ptr<ypc::key_node<secp256k1_skey_group>>> children1 =
      {std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[0]),
       std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[1]),
       std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[2])};
  auto sub_root1 = std::make_shared<ypc::key_node<secp256k1_skey_group>>();
  ypc::kgt_json<secp256k1_skey_group> k1(sub_root1, children1);
  return_val = k1.calculate_kgt_sum();

  std::vector<std::shared_ptr<ypc::key_node<secp256k1_skey_group>>> children2 =
      {std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[3]),
       std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[4]),
       std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[5])};
  auto sub_root2 = std::make_shared<ypc::key_node<secp256k1_skey_group>>();
  ypc::kgt_json<secp256k1_skey_group> k2(sub_root2, children2);
  return_val = k2.calculate_kgt_sum();

  children1.clear();
  children1.push_back(k1.root());
  children1.push_back(
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[6]));
  children1.push_back(
      std::make_shared<ypc::key_node<secp256k1_skey_group>>(key_nodes[7]));
  children1.push_back(k2.root());
  auto root = std::make_shared<ypc::key_node<secp256k1_skey_group>>();
  ypc::kgt_json<secp256k1_skey_group> k3(root, children1);
  return_val = k3.calculate_kgt_sum();

  skey_t root_sum;
  return_val = secp256k1_skey_group::add(root_sum, k1.sum(), skey_vec[6]);
  return_val = secp256k1_skey_group::add(root_sum, root_sum, skey_vec[7]);
  return_val = secp256k1_skey_group::add(root_sum, root_sum, k2.sum());
  EXPECT_TRUE(memcmp(&root_sum, &k3.sum(), 32) == 0);

  stbox::bytes res = k3.to_bytes();
  ypc::kgt_json<secp256k1_skey_group> k4(res);
  stbox::bytes res2 = k4.to_bytes();
  EXPECT_EQ(memcmp(res.data(), res2.data(), res.size()), 0);
}*/
