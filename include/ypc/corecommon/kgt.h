#pragma once
#include "crypto/group.h"
#include "crypto/group_traits.h"
#include "nt_cols.h"
#include "package.h"
#include "ypc/stbox/ebyte.h"
#include <memory>
#include <queue>
#include <unordered_map>

namespace ypc {
typedef stbox::bytes kgt_bytes_t;
typedef nt<kgt_bytes_t> ntb;

template <typename Group> struct key_node {
  typedef Group group_t;
  typedef typename Group::key_t group_key_t;

  // value
  group_key_t key_val;
  // children
  std::vector<std::shared_ptr<key_node<group_t>>> children;
  key_node() {}
  key_node(const group_key_t &val) : key_val(val) {}
};
template <typename Group> class kgt {
protected:
  typedef Group group_t;
  typedef typename group_t::key_t group_key_t;
  typedef typename ypc::crypto::ecc_traits<group_t>::ecc_t ecc_t;
  typedef typename ypc::crypto::ecc_traits<group_t>::peer_group_t peer_group_t;
  typedef typename peer_group_t::key_t group_peer_key_t;

public:
  kgt(const std::shared_ptr<key_node<group_t>> &root_node) {
    m_root = root_node;
  }

  kgt(const std::shared_ptr<key_node<group_t>> &root_node,
      const std::vector<std::shared_ptr<key_node<group_t>>> &key_nodes) {
    m_root = root_node;
    for (auto &key_node : key_nodes) {
      m_root->children.push_back(key_node);
    }
  }

  kgt(const kgt_bytes_t &bytes_kgt) { m_root = b_deserialize_node(bytes_kgt); }

  kgt_bytes_t to_bytes() { return b_serialize_node(*m_root); }

  int calculate_kgt_sum() {
    m_sum = calculate_node_sum(*m_root);
    return 1;
  }

public:
  std::shared_ptr<key_node<peer_group_t>> gen_pkey_kgt_from_skey_kgt(
      const std::shared_ptr<key_node<group_t>> &skey_kgt) {
    static_assert(ypc::crypto::ecc_traits<group_t>::is_private_key_type,
                  "invalid ecc key group, only supported for "
                  "secp256k1_skey_group or sm2_skey_group");
    auto pkey_kgt = std::make_shared<key_node<peer_group_t>>();
    if (skey_kgt->children.empty()) {
      ecc_t::generate_pkey_from_skey(
          (uint8_t *)&skey_kgt->key_val, sizeof(skey_kgt->key_val),
          (uint8_t *)&pkey_kgt->key_val, sizeof(pkey_kgt->key_val));
      return pkey_kgt;
    }
    for (const auto &child : skey_kgt->children) {
      pkey_kgt->children.push_back(gen_pkey_kgt_from_skey_kgt(child));
    }
    return pkey_kgt;
  }

  std::shared_ptr<key_node<peer_group_t>> construct_skey_kgt_with_pkey_kgt(
      const std::shared_ptr<key_node<group_t>> &pkey_kgt,
      const std::unordered_map<kgt_bytes_t, kgt_bytes_t> &peer) {
    static_assert(!ypc::crypto::ecc_traits<group_t>::is_private_key_type,
                  "invalid ecc key group, only supported for "
                  "secp256k1_pkey_group or sm2_pkey_group");
    auto skey_kgt = std::make_shared<key_node<peer_group_t>>();
    if (pkey_kgt->children.empty()) {
      kgt_bytes_t pk((uint8_t *)&pkey_kgt->key_val, sizeof(pkey_kgt->key_val));
      const auto &it = peer.find(pk);
      if (it != peer.end()) {
        const auto &sk = it->second;
        memcpy(skey_kgt->key_val.data, sk.data(), sk.size());
      }
      return skey_kgt;
    }
    for (const auto &child : pkey_kgt->children) {
      skey_kgt->children.push_back(
          construct_skey_kgt_with_pkey_kgt(child, peer));
    }
    return skey_kgt;
  }

  std::vector<std::shared_ptr<key_node<group_t>>> leaves() {
    std::vector<std::shared_ptr<key_node<group_t>>> v;
    std::queue<std::shared_ptr<key_node<group_t>>> q;
    q.push(m_root);
    while (!q.empty()) {
      auto &ele = q.front();
      q.pop();
      if (ele->children.empty()) {
        v.push_back(ele);
        continue;
      }
      for (auto &child : ele->children) {
        q.push(child);
      }
    }
    return v;
  }

public:
  const group_key_t &sum() const { return m_sum; }
  const std::shared_ptr<key_node<group_t>> &root() const { return m_root; }

protected:
  group_key_t calculate_node_sum(const key_node<group_t> &node) {
    if (node.children.empty()) {
      return node.key_val;
    }
    group_key_t sum = calculate_node_sum(*node.children.back());
    for (int i = 0; i < node.children.size() - 1; i++) {
      auto child_sum = calculate_node_sum(*node.children[i]);
      group_t::add(sum, sum, child_sum);
    }
    return sum;
  }

  kgt_bytes_t b_serialize_node(const key_node<group_t> &node) {
    ntb::kgt_pkg_t pkg;
    kgt_bytes_t val((uint8_t *)&node.key_val, sizeof(group_key_t));
    pkg.set<ntb::kgt_value>(val);
    std::vector<kgt_bytes_t> v;
    for (const auto &child : node.children) {
      auto b_child = b_serialize_node(*child);
      v.push_back(b_child);
    }
    pkg.set<ntb::kgt_children>(v);
    return ypc::make_bytes<kgt_bytes_t>::for_package(pkg);
  }

  std::shared_ptr<key_node<group_t>> b_deserialize_node(const kgt_bytes_t &b) {
    auto ptr = std::make_shared<key_node<group_t>>();
    auto pkg = ypc::make_package<ntb::kgt_pkg_t>::from_bytes(b);
    auto val = pkg.get<ntb::kgt_value>();
    memcpy(&ptr->key_val, val.data(), sizeof(group_key_t));
    for (const auto &child : pkg.get<ntb::kgt_children>()) {
      ptr->children.push_back(b_deserialize_node(child));
    }
    return ptr;
  }

protected:
  group_key_t m_sum;
  std::shared_ptr<key_node<group_t>> m_root;
};
} // namespace ypc
