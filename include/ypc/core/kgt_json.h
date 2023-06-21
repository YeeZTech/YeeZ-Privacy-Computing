#pragma once
#include "ypc/corecommon/kgt.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace ypc {
typedef ypc::bytes kgt_json_bytes_t;
typedef nt<kgt_json_bytes_t> ntb_json;

template <typename Group> class kgt_json {
  typedef Group group_t;
  typedef typename Group::key_t group_key_t;

public:
  kgt_json(const std::shared_ptr<key_node<group_t>> &root_node,
           const std::vector<std::shared_ptr<key_node<group_t>>> &key_nodes) {
    m_root = root_node;
    for (auto &key_node : key_nodes) {
      m_root->children.push_back(key_node);
    }
  }

  kgt_json(const std::string &json_kgt) {
    boost::property_tree::ptree pt;
    std::stringstream ss(json_kgt);
    boost::property_tree::json_parser::read_json(ss, pt);
    m_root = deserialize_node(pt);
  }

  kgt_json(const kgt_bytes_t &bytes_kgt) {
    m_root = b_deserialize_node(bytes_kgt);
  }

  std::string to_string() {
    std::stringstream ss;
    auto pt = seriliaze_node(*m_root);
    boost::property_tree::json_parser::write_json(ss, pt);
    return ss.str();
  }

  kgt_bytes_t to_bytes() { return b_serialize_node(*m_root); }

  int calculate_kgt_sum() {
    m_sum = calculate_node_sum(*m_root);
    return 1;
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

  boost::property_tree::ptree seriliaze_node(const key_node<group_t> &node) {
    std::stringstream ss;
    kgt_json_bytes_t val((uint8_t *)&node.key_val, sizeof(group_key_t));
    ss << val;
    boost::property_tree::ptree pt;
    pt.put("value", ss.str());
    boost::property_tree::ptree arr;
    for (const auto &child : node.children) {
      auto child_pt = seriliaze_node(*child);
      arr.push_back(boost::property_tree::ptree::value_type("", child_pt));
    }
    pt.add_child("children", arr);
    return pt;
  }

  std::shared_ptr<key_node<group_t>>
  deserialize_node(const boost::property_tree::ptree &pt) {
    auto ptr = std::make_shared<key_node<group_t>>();
    auto bval =
        ypc::hex_bytes(pt.get<std::string>("value")).as<kgt_json_bytes_t>();
    memcpy(&ptr->key_val, bval.data(), sizeof(group_key_t));
    for (const auto &ele : pt.get_child("children")) {
      ptr->children.push_back(deserialize_node(ele.second));
    }
    return ptr;
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
