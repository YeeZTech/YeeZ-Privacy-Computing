#include "onchain_data_reader.h"
#include "ypc/filesystem.h"
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <stbox/ebyte.h>

onchain_data_reader::~onchain_data_reader() {}

const onchain_data_meta_t &
onchain_data_reader::get_data_with_hash(const std::string &hash) {
  if (m_data_index.empty()) {
    for (size_t i = 0; i < m_all_data.size(); ++i) {
      m_data_index.insert(std::make_pair(m_all_data[i].get<data_hash>(), i));
    }
  }
  return m_all_data[m_data_index[hash]];
}

dummy_onchain_data_reader::dummy_onchain_data_reader(const std::string &path)
    : m_sample_path(path) {}

void dummy_onchain_data_reader::init() {
  m_all_data.clear();

  if (!ypc::is_file_exists(m_sample_path)) {
    throw std::runtime_error(
        boost::str(boost::format("file %1% not exist") % m_sample_path));
  }

  try {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(m_sample_path, pt);
    BOOST_FOREACH (boost::property_tree::ptree::value_type &data,
                   pt.get_child("data")) {
      onchain_data_meta_t d;
      auto dhash = stbox::byte_to_string(
          stbox::bytes::from_hex(data.second.get<std::string>("data-hash")));
      auto pkey = stbox::byte_to_string(stbox::bytes::from_hex(
          data.second.get<std::string>("provider-pkey")));
      d.set<data_hash, provider_pub_key>(dhash, pkey);
      m_all_data.push_back(d);
    }
  } catch (const std::exception &e) {
    throw std::runtime_error(
        boost::str(boost::format("read json failed! Error: %1%") % e.what()));
  }
}
