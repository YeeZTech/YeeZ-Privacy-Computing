#include "./extra_data_source.h"
#include "ypc/byte.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "./extra_data_source_reader.h"
#include <boost/foreach.hpp>

namespace ypc {

extra_data_source_t
read_extra_data_source_from_file(const std::string &_file_path) {
  extra_data_source_t ret;
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(_file_path, pt);

    BOOST_FOREACH (boost::property_tree::ptree::value_type &v, pt) {
      std::string name = v.second.get<std::string>("name");
      std::cout << "name: " << name << std::endl;
      extra_data_group_t g;
      g.set<extra_data_group_name>(name);
      std::vector<extra_data_item_t> items;
      BOOST_FOREACH (boost::property_tree::ptree::value_type &g,
                     v.second.get_child("data")) {
        extra_data_item_t item;
        std::string url = g.second.get<std::string>("file-path");
        ypc::bytes _data_hash =
            g.second.get<ypc::hex_bytes>("data-hash").as<ypc::bytes>();
        ypc::bytes license =
            g.second.get<ypc::hex_bytes>("usage-license").as<ypc::bytes>();
        item.set<file_path>(url);
        item.set<data_use_license>(license);
        item.set<data_hash>(_data_hash);
        items.push_back(item);
        // std::cout << url << ", " << data_hash << ", " << license <<
        // std::endl;
      }
      g.set<extra_data_set>(items);
      ret.push_back(g);
    }
    return ret;
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format("Read forward params from file failed! Error: %1%") %
        e.what()));
    return ret;
  }
}

void write_extra_data_source_to_file(const extra_data_source_t _data_source,
                                     const std::string &file_path) {}

} // namespace ypc

std::shared_ptr<extra_data_source_reader> g_data_source_reader;

uint32_t ocall_read_next_extra_data_item(uint8_t *data_hash,
                                         uint32_t hash_size) {
  return g_data_source_reader->next_extra_data_item(data_hash, hash_size);
}
uint32_t ocall_get_next_extra_data_item_size() {
  return g_data_source_reader->get_next_extra_data_item_size();
}
uint32_t ocall_get_next_extra_data_item_data(uint8_t *item_data,
                                             uint32_t ndi_size) {
  return g_data_source_reader->get_next_extra_data_item_data(item_data,
                                                             ndi_size);
}
