#include <limits.h>
#include <stddef.h>

#include "ypc/corecommon/crypto/group.h"
#include "ypc/terminus/crypto_pack.h"
#include "ypc/corecommon/crypto/crypto_pack.h"
#include "ypc/core/byte.h"
#include "ypc/core/kgt_json.h"
#include <assert.h>
#include <gtest/gtest.h>
#include <iostream>
#include <glog/logging.h>

TEST(test_secp256k1pri_kgtjson, create_kgt_no_child) {
  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  ypc::crypto::secp256k1_skey_group::key_t skey1;
  int return_val;

  auto skey1_b = crypto->gen_ecc_private_key();
  memcpy(&skey1, skey1_b.data(), sizeof(skey1));
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
  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  ypc::crypto::secp256k1_skey_group::key_t skey1, skey2, skey3, skey4;
  int return_val;

  auto skey1_b = crypto->gen_ecc_private_key();
  auto skey2_b = crypto->gen_ecc_private_key();
  auto skey3_b = crypto->gen_ecc_private_key();
  auto skey4_b = crypto->gen_ecc_private_key();
  memcpy(&skey1, skey1_b.data(), sizeof(skey1));
  memcpy(&skey2, skey2_b.data(), sizeof(skey2));
  memcpy(&skey3, skey3_b.data(), sizeof(skey3));
  memcpy(&skey4, skey4_b.data(), sizeof(skey4));

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

TEST(test_secp256k1pri_kgtjson, skey_pkey_kgt_conversion) {
  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  int return_val;

  ypc::crypto::secp256k1_skey_group::key_t skey1;
  auto skey1_b = crypto->gen_ecc_private_key();
  memcpy(&skey1, skey1_b.data(), sizeof(skey1));
  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey1_node(skey1);
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> children;
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey1_node),
      children);
  k.calculate_kgt_sum();
  const auto &skey_sum=k.sum();
  ypc::bytes skey_sum_b(skey_sum.data, 32);
  LOG(INFO) << skey_sum_b;
  auto pkey_sum_from_skey = crypto->gen_ecc_public_key_from_private_key(skey_sum_b);
  LOG(INFO) << pkey_sum_from_skey;

  auto pkey_node = k.gen_pkey_kgt_from_skey_kgt(k.root());
  ypc::kgt_json<ypc::crypto::secp256k1_pkey_group> pkey_kgt(pkey_node);
  pkey_kgt.calculate_kgt_sum();
  const auto &pkey_sum=pkey_kgt.sum();
  ypc::bytes pkey_sum_b(pkey_sum.data, sizeof(pkey_sum));
  LOG(INFO) << pkey_sum_b;
  //std::unordered_map<stbox::bytes, stbox::bytes> peer;
  //std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_pkey_group>>> pkey_kgt_leaves = pkey_kgt.leaves();
  //std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> skey_kgt_leaves = k.leaves();
  //for(int i = 0; i < pkey_kgt_leaves.size(); i++){
    //stbox::bytes data_pkey_b((uint8_t *)&pkey_kgt_leaves[i]->key_val, sizeof(pkey_kgt_leaves[i]->key_val));
    //stbox::bytes data_skey_b((uint8_t *)&skey_kgt_leaves[i]->key_val, sizeof(skey_kgt_leaves[i]->key_val));
    //if (peer.find(data_pkey_b) != peer.end()) {
      //peer.insert(std::make_pair(data_pkey_b, data_skey_b));
    //}
  //}
  //auto skey_kgt_node = ypc::kgt_json<ypc::crypto::secp256k1_pkey_group>::construct_skey_kgt_with_pkey_kgt(pkey_node, peer);
  //ypc::kgt_json<ypc::crypto::secp256k1_skey_group> skey_kgt_copy(skey_kgt_node);
  //std::string res = k.to_string();
  //std::string res2 = skey_kgt_copy.to_string();
  EXPECT_EQ(memcmp(pkey_sum_b.data(), pkey_sum_from_skey.data(), sizeof(pkey_sum_b)), 0);
}

TEST(test_secp256k1pri_kgtjson, construct_skey_kgt) {
  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  int return_val;

  ypc::crypto::secp256k1_skey_group::key_t skey1;
  auto skey1_b = crypto->gen_ecc_private_key();
  memcpy(&skey1, skey1_b.data(), sizeof(skey1));
  ypc::key_node<ypc::crypto::secp256k1_skey_group> skey1_node(skey1);
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> children;
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(skey1_node),
      children);
  LOG(INFO) << k.to_string();
  k.calculate_kgt_sum();
  auto pkey_node = k.gen_pkey_kgt_from_skey_kgt(k.root());
  ypc::kgt_json<ypc::crypto::secp256k1_pkey_group> pkey_kgt(pkey_node);

  std::unordered_map<stbox::bytes, stbox::bytes> peer;
  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> skey_kgt_leaves = k.leaves();
  for (auto&l:skey_kgt_leaves){
    ypc::bytes ls(l->key_val.data, 32);
    auto lp=crypto->gen_ecc_public_key_from_private_key(ls);
    peer.insert(std::make_pair(lp, ls));
  }
  //for(int i = 0; i < pkey_kgt_leaves.size(); i++){
    //stbox::bytes data_pkey_b((uint8_t *)&pkey_kgt_leaves[i]->key_val, sizeof(pkey_kgt_leaves[i]->key_val));
    //stbox::bytes data_skey_b((uint8_t *)&skey_kgt_leaves[i]->key_val, sizeof(skey_kgt_leaves[i]->key_val));
    //if (peer.find(data_pkey_b) != peer.end()) {
      //peer.insert(std::make_pair(data_pkey_b, data_skey_b));
    //}
  //}
  auto skey_kgt_node = ypc::kgt_json<ypc::crypto::secp256k1_pkey_group>::construct_skey_kgt_with_pkey_kgt(pkey_node, peer);
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> skey_kgt_copy(skey_kgt_node);
  LOG(INFO) << skey_kgt_copy.to_string();
  //std::string res = k.to_string();
  //std::string res2 = skey_kgt_copy.to_string();
  //EXPECT_EQ(memcmp(pkey_sum_b.data(), pkey_sum_from_skey.data(), sizeof(pkey_sum_b)), 0);
}
/*
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
*/
TEST(test_secp256k1pri_kgtjson, complex_kgt) {
  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  using skey_t = ypc::crypto::secp256k1_skey_group::key_t;
  std::vector<skey_t> skey_vec;
  skey_t tmp_skey;
  std::vector<ypc::key_node<ypc::crypto::secp256k1_skey_group>> key_nodes;
  int return_val;

  for (int i = 0; i < 8; i++) {
    auto skey_b = crypto->gen_ecc_private_key();
    memcpy(&tmp_skey, skey_b.data(),sizeof(tmp_skey));
    skey_vec.emplace_back(tmp_skey);
    key_nodes.emplace_back(skey_vec[i]);
  }

  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> children1 =
      {std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[0]),
       std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[1]),
       std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[2])};
  auto sub_root1 = std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>();
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k1(sub_root1, children1);
  return_val = k1.calculate_kgt_sum();

  std::vector<std::shared_ptr<ypc::key_node<ypc::crypto::secp256k1_skey_group>>> children2 =
      {std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[3]),
       std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[4]),
       std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[5])};
  auto sub_root2 = std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>();
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k2(sub_root2, children2);
  return_val = k2.calculate_kgt_sum();

  children1.clear();
  children1.push_back(k1.root());
  children1.push_back(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[6]));
  children1.push_back(
      std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>(key_nodes[7]));
  children1.push_back(k2.root());
  auto root = std::make_shared<ypc::key_node<ypc::crypto::secp256k1_skey_group>>();
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k3(root, children1);
  return_val = k3.calculate_kgt_sum();

  const auto &skey_sum=k3.sum();
  ypc::bytes skey_sum_b(skey_sum.data, 32);
  LOG(INFO) << skey_sum_b;
  auto pkey_sum_from_skey = crypto->gen_ecc_public_key_from_private_key(skey_sum_b);
  LOG(INFO) << pkey_sum_from_skey;

  auto pkey_node = k3.gen_pkey_kgt_from_skey_kgt(k3.root());
  ypc::kgt_json<ypc::crypto::secp256k1_pkey_group> pkey_kgt(pkey_node);
  pkey_kgt.calculate_kgt_sum();
  const auto &pkey_sum=pkey_kgt.sum();
  ypc::bytes pkey_sum_b(pkey_sum.data, sizeof(pkey_sum));
  LOG(INFO) << pkey_sum_b;
  EXPECT_EQ(memcmp(pkey_sum_b.data(), pkey_sum_from_skey.data(), sizeof(pkey_sum_b)), 0);

  skey_t root_sum;
  return_val = ypc::crypto::secp256k1_skey_group::add(root_sum, k1.sum(), skey_vec[6]);
  return_val = ypc::crypto::secp256k1_skey_group::add(root_sum, root_sum, skey_vec[7]);
  return_val = ypc::crypto::secp256k1_skey_group::add(root_sum, root_sum, k2.sum());
  EXPECT_TRUE(memcmp(&root_sum, &k3.sum(), 32) == 0);

  stbox::bytes res = k3.to_bytes();
  ypc::kgt_json<ypc::crypto::secp256k1_skey_group> k4(res);
  stbox::bytes res2 = k4.to_bytes();
  EXPECT_EQ(memcmp(res.data(), res2.data(), res.size()), 0);
}
