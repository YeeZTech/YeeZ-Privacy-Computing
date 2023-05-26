#include "ypc/core/group.h"
#include "ypc/core/kgt.h"
#include <assert.h>
#include <gmssl/sm2.h>
#include <gtest/gtest.h>

TEST(test_sm2pri_kgt, create_kgt_no_child) {
  SM2_KEY sm2_key1;
  int return_val = sm2_key_generate(&sm2_key1);
  sm2_skey_group::key_t skey1;
  memcpy(&skey1, sm2_key1.private_key, 32);
  key_node<sm2_skey_group> skey1_node(skey1);
  std::vector<std::shared_ptr<key_node<sm2_skey_group>>> children;
  kgt<sm2_skey_group> k(std::make_shared<key_node<sm2_skey_group>>(skey1_node),
                        children);
  std::string res = k.to_string();
  std::cout << res << std::endl;
  kgt<sm2_skey_group> k2(res);
  std::string res2 = k2.to_string();
  std::cout << res2 << std::endl;
  EXPECT_EQ(res, res2);
}

TEST(test_sm2pri_kgt, create_kgt_using_node) {
  SM2_KEY sm2_key1, sm2_key2, sm2_key3, sm2_key4;
  sm2_skey_group::key_t skey1, skey2, skey3, skey4;
  int return_val = sm2_key_generate(&sm2_key1);
  return_val = sm2_key_generate(&sm2_key2);
  return_val = sm2_key_generate(&sm2_key3);
  return_val = sm2_key_generate(&sm2_key4);
  memcpy(&skey1, sm2_key1.private_key, 32);
  memcpy(&skey2, sm2_key2.private_key, 32);
  memcpy(&skey3, sm2_key3.private_key, 32);
  memcpy(&skey4, sm2_key4.private_key, 32);

  key_node<sm2_skey_group> skey1_node(skey1);
  key_node<sm2_skey_group> skey2_node(skey2);
  key_node<sm2_skey_group> skey3_node(skey3);
  key_node<sm2_skey_group> skey4_node(skey4);
  std::vector<std::shared_ptr<key_node<sm2_skey_group>>> children = {
      std::make_shared<key_node<sm2_skey_group>>(skey2_node),
      std::make_shared<key_node<sm2_skey_group>>(skey3_node)};
  kgt<sm2_skey_group> k1(std::make_shared<key_node<sm2_skey_group>>(skey1_node),
                         children);

  children.clear();
  children.push_back(k1.root());
  kgt<sm2_skey_group> k4(std::make_shared<key_node<sm2_skey_group>>(skey4_node),
                         children);
  std::string res = k4.to_string();
  std::cout << res << std::endl;
  kgt<sm2_skey_group> k2(res);
  std::string res2 = k2.to_string();
  std::cout << res2 << std::endl;
  EXPECT_EQ(res, res2);
}
TEST(test_sm2pri_kgt, calculate_sum_no_child) {
  SM2_KEY sm2_key1;
  sm2_skey_group::key_t skey1;
  int return_val = sm2_key_generate(&sm2_key1);
  memcpy(&skey1, sm2_key1.private_key, 32);

  key_node<sm2_skey_group> skey1_node(skey1);
  std::vector<std::shared_ptr<key_node<sm2_skey_group>>> children;
  kgt<sm2_skey_group> k(std::make_shared<key_node<sm2_skey_group>>(skey1_node),
                        children);
  return_val = k.calculate_kgt_sum();
  EXPECT_TRUE(memcmp(&sm2_key1.private_key, &k.sum(),
                     sizeof(sm2_key1.private_key)) == 0);
}
TEST(test_sm2pri_kgt, calculate_sum_one_child) {
  SM2_KEY sm2_key1, sm2_key2;
  sm2_skey_group::key_t skey1, skey2;
  int return_val = sm2_key_generate(&sm2_key1);
  return_val = sm2_key_generate(&sm2_key2);
  memcpy(&skey1, sm2_key1.private_key, 32);
  memcpy(&skey2, sm2_key2.private_key, 32);
  key_node<sm2_skey_group> skey1_node(skey1);
  key_node<sm2_skey_group> skey2_node(skey2);
  std::vector<std::shared_ptr<key_node<sm2_skey_group>>> children = {
      std::make_shared<key_node<sm2_skey_group>>(skey2_node)};
  kgt<sm2_skey_group> k(std::make_shared<key_node<sm2_skey_group>>(skey1_node),
                        children);
  return_val = k.calculate_kgt_sum();
  EXPECT_TRUE(memcmp(&skey2, &k.sum(), sizeof(sm2_skey_group::key_t)) == 0);
}
TEST(test_sm2pri_kgt, calculate_sum_multi_children) {
  SM2_KEY sm2_key1, sm2_key2, sm2_key3;
  sm2_skey_group::key_t skey1, skey2, skey3;
  int return_val;
  return_val = sm2_key_generate(&sm2_key1);
  return_val = sm2_key_generate(&sm2_key2);
  return_val = sm2_key_generate(&sm2_key3);
  memcpy(&skey1, sm2_key1.private_key, 32);
  memcpy(&skey2, sm2_key2.private_key, 32);
  memcpy(&skey3, sm2_key3.private_key, 32);

  key_node<sm2_skey_group> skey1_node(skey1);
  key_node<sm2_skey_group> skey2_node(skey2);
  key_node<sm2_skey_group> skey3_node(skey3);
  std::vector<std::shared_ptr<key_node<sm2_skey_group>>> children = {
      std::make_shared<key_node<sm2_skey_group>>(skey2_node),
      std::make_shared<key_node<sm2_skey_group>>(skey3_node)};
  kgt<sm2_skey_group> k(std::make_shared<key_node<sm2_skey_group>>(skey1_node),
                        children);
  return_val = k.calculate_kgt_sum();
  sm2_skey_group::key_t res;
  return_val = sm2_skey_group::add(res, skey2, skey3);
  EXPECT_TRUE(memcmp(&res, &k.sum(), sizeof(sm2_skey_group::key_t)) == 0);
}

TEST(test_sm2pri_kgt, complex_kgt) {
  using skey_t = sm2_skey_group::key_t;
  std::vector<skey_t> skey_vec;
  std::vector<SM2_KEY> sm2key_vec;
  skey_t tmp_skey;
  std::vector<key_node<sm2_skey_group>> key_nodes;
  int return_val;
  for (int i = 0; i < 8; i++) {
    sm2key_vec.emplace_back();
    return_val = sm2_key_generate(&sm2key_vec[i]);
    memcpy(&tmp_skey, sm2key_vec[i].private_key, 32);
    skey_vec.emplace_back(tmp_skey);
    key_nodes.emplace_back(skey_vec[i]);
  }

  std::vector<std::shared_ptr<key_node<sm2_skey_group>>> children1 = {
      std::make_shared<key_node<sm2_skey_group>>(key_nodes[0]),
      std::make_shared<key_node<sm2_skey_group>>(key_nodes[1]),
      std::make_shared<key_node<sm2_skey_group>>(key_nodes[2])};
  auto sub_root1 = std::make_shared<key_node<sm2_skey_group>>();
  kgt<sm2_skey_group> k1(sub_root1, children1);
  return_val = k1.calculate_kgt_sum();

  std::vector<std::shared_ptr<key_node<sm2_skey_group>>> children2 = {
      std::make_shared<key_node<sm2_skey_group>>(key_nodes[3]),
      std::make_shared<key_node<sm2_skey_group>>(key_nodes[4]),
      std::make_shared<key_node<sm2_skey_group>>(key_nodes[5])};
  auto sub_root2 = std::make_shared<key_node<sm2_skey_group>>();
  kgt<sm2_skey_group> k2(sub_root2, children2);
  return_val = k2.calculate_kgt_sum();

  children1.clear();
  children1.push_back(k1.root());
  children1.push_back(std::make_shared<key_node<sm2_skey_group>>(key_nodes[6]));
  children1.push_back(std::make_shared<key_node<sm2_skey_group>>(key_nodes[7]));
  children1.push_back(k2.root());
  auto root = std::make_shared<key_node<sm2_skey_group>>();
  kgt<sm2_skey_group> k3(root, children1);
  return_val = k3.calculate_kgt_sum();

  skey_t root_sum;
  return_val = sm2_skey_group::add(root_sum, k1.sum(), skey_vec[6]);
  return_val = sm2_skey_group::add(root_sum, root_sum, skey_vec[7]);
  return_val = sm2_skey_group::add(root_sum, root_sum, k2.sum());
  EXPECT_TRUE(memcmp(&root_sum, &k3.sum(), 32) == 0);

  std::string res = k3.to_string();
  kgt<sm2_skey_group> k4(res);
  std::string res2 = k4.to_string();
  EXPECT_EQ(res, res2);
}
