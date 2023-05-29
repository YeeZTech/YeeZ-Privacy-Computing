#include "ypc/corecommon/crypto/group.h"
#include "ypc/corecommon/kgt.h"
#include <assert.h>
#include <gmssl/sm2.h>
#include <gtest/gtest.h>

TEST(test_sm2pub_kgt, create_kgt_no_child) {
  SM2_KEY sm2_key1;
  int return_val = sm2_key_generate(&sm2_key1);

  ypc::key_node<sm2_pkey_group> pkey1_node(sm2_key1.public_key);
  std::vector<std::shared_ptr<ypc::key_node<sm2_pkey_group>>> children;
  ypc::kgt<sm2_pkey_group> k(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey1_node), children);
  stbox::bytes res = k.to_bytes();
  ypc::kgt<sm2_pkey_group> k2(res);
  stbox::bytes res2 = k2.to_bytes();
  EXPECT_EQ(memcmp(res.data(), res2.data(), res.size()), 0);
}

TEST(test_sm2pub_kgt, create_kgt_using_node) {
  SM2_KEY sm2_key1, sm2_key2, sm2_key3, sm2_key4;
  int return_val = sm2_key_generate(&sm2_key1);
  return_val = sm2_key_generate(&sm2_key2);
  return_val = sm2_key_generate(&sm2_key3);
  return_val = sm2_key_generate(&sm2_key4);

  ypc::key_node<sm2_pkey_group> pkey1_node(sm2_key1.public_key);
  ypc::key_node<sm2_pkey_group> pkey2_node(sm2_key2.public_key);
  ypc::key_node<sm2_pkey_group> pkey3_node(sm2_key3.public_key);
  ypc::key_node<sm2_pkey_group> pkey4_node(sm2_key4.public_key);
  std::vector<std::shared_ptr<ypc::key_node<sm2_pkey_group>>> children = {
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey2_node),
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey3_node)};
  ypc::kgt<sm2_pkey_group> k1(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey1_node), children);

  children.clear();
  children.push_back(k1.root());
  ypc::kgt<sm2_pkey_group> k4(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey4_node), children);
  stbox::bytes res = k4.to_bytes();
  ypc::kgt<sm2_pkey_group> k2(res);
  stbox::bytes res2 = k2.to_bytes();
  EXPECT_EQ(memcmp(res.data(), res2.data(), res.size()), 0);
}

TEST(test_sm2pub_kgt, calculate_sum_no_child) {
  SM2_KEY sm2_key1;
  int return_val = sm2_key_generate(&sm2_key1);

  ypc::key_node<sm2_pkey_group> pkey1_node(sm2_key1.public_key);
  std::vector<std::shared_ptr<ypc::key_node<sm2_pkey_group>>> children;
  ypc::kgt<sm2_pkey_group> k(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey1_node), children);
  return_val = k.calculate_kgt_sum();
  EXPECT_TRUE(
      memcmp(&sm2_key1.public_key, &k.sum(), sizeof(sm2_key1.public_key)) == 0);
}

TEST(test_sm2pub_kgt, calculate_sum_one_child) {
  SM2_KEY sm2_key1, sm2_key2;
  int return_val = sm2_key_generate(&sm2_key1);
  return_val = sm2_key_generate(&sm2_key2);
  ypc::key_node<sm2_pkey_group> pkey1_node(sm2_key1.public_key);
  ypc::key_node<sm2_pkey_group> pkey2_node(sm2_key2.public_key);
  std::vector<std::shared_ptr<ypc::key_node<sm2_pkey_group>>> children = {
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey2_node)};
  ypc::kgt<sm2_pkey_group> k(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey1_node), children);
  return_val = k.calculate_kgt_sum();
  EXPECT_TRUE(memcmp(&sm2_key2.public_key, &k.sum(), sizeof(SM2_POINT)) == 0);
}

TEST(test_sm2pub_kgt, calculate_sum_multi_children) {
  SM2_KEY sm2_key1, sm2_key2, sm2_key3;
  int return_val;
  return_val = sm2_key_generate(&sm2_key1);
  return_val = sm2_key_generate(&sm2_key2);
  return_val = sm2_key_generate(&sm2_key3);

  ypc::key_node<sm2_pkey_group> pkey1_node(sm2_key1.public_key);
  ypc::key_node<sm2_pkey_group> pkey2_node(sm2_key2.public_key);
  ypc::key_node<sm2_pkey_group> pkey3_node(sm2_key3.public_key);
  std::vector<std::shared_ptr<ypc::key_node<sm2_pkey_group>>> children = {
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey2_node),
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey3_node)};
  ypc::kgt<sm2_pkey_group> k(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(pkey1_node), children);
  return_val = k.calculate_kgt_sum();
  SM2_POINT res;
  return_val =
      sm2_pkey_group::add(res, sm2_key3.public_key, sm2_key2.public_key);
  EXPECT_TRUE(memcmp(&res, &k.sum(), sizeof(sm2_pkey_group::key_t)) == 0);
}

TEST(test_sm2pub_kgt, complex_kgt) {
  std::vector<SM2_KEY> sm2key_vec;
  std::vector<ypc::key_node<sm2_pkey_group>> key_nodes;
  int return_val;
  for (int i = 0; i < 8; i++) {
    sm2key_vec.emplace_back();
    return_val = sm2_key_generate(&sm2key_vec[i]);
    key_nodes.emplace_back(sm2key_vec[i].public_key);
  }

  std::vector<std::shared_ptr<ypc::key_node<sm2_pkey_group>>> children1 = {
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[0]),
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[1]),
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[2])};
  auto sub_root1 = std::make_shared<ypc::key_node<sm2_pkey_group>>();
  ypc::kgt<sm2_pkey_group> k1(sub_root1, children1);
  return_val = k1.calculate_kgt_sum();

  std::vector<std::shared_ptr<ypc::key_node<sm2_pkey_group>>> children2 = {
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[3]),
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[4]),
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[5])};
  auto sub_root2 = std::make_shared<ypc::key_node<sm2_pkey_group>>();
  ypc::kgt<sm2_pkey_group> k2(sub_root2, children2);
  return_val = k2.calculate_kgt_sum();

  children1.clear();
  children1.push_back(k1.root());
  children1.push_back(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[6]));
  children1.push_back(
      std::make_shared<ypc::key_node<sm2_pkey_group>>(key_nodes[7]));
  children1.push_back(k2.root());
  auto root = std::make_shared<ypc::key_node<sm2_pkey_group>>();
  ypc::kgt<sm2_pkey_group> k3(root, children1);
  return_val = k3.calculate_kgt_sum();

  SM2_POINT root_sum;
  return_val =
      sm2_pkey_group::add(root_sum, k1.sum(), sm2key_vec[6].public_key);
  return_val =
      sm2_pkey_group::add(root_sum, root_sum, sm2key_vec[7].public_key);
  return_val = sm2_pkey_group::add(root_sum, root_sum, k2.sum());
  EXPECT_TRUE(memcmp(&root_sum, &k3.sum(), 32) == 0);

  stbox::bytes res = k3.to_bytes();
  ypc::kgt<sm2_pkey_group> k4(res);
  stbox::bytes res2 = k4.to_bytes();
  EXPECT_EQ(memcmp(res.data(), res2.data(), res.size()), 0);
}
