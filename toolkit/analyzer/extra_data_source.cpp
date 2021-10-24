#include "./extra_data_source.h"
#include "ypc/byte.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <boost/foreach.hpp>
namespace ypc {

extra_data_source_t
read_extra_data_source_from_file(const std::string &file_path) {
  extra_data_source_t ret;
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(file_path, pt);

    BOOST_FOREACH (boost::property_tree::ptree::value_type &v, pt) {
      std::string name = v.second.get<std::string>("name");
      std::cout << "name: " << name << std::endl;
      BOOST_FOREACH (boost::property_tree::ptree::value_type &g,
                     v.second.get_child("data")) {
        std::string url = g.second.get<std::string>("file-path");
        ypc::bytes data_hash = g.second.get<ypc::bytes>("data-hash");
        ypc::bytes license = g.second.get<ypc::bytes>("usage-license");
        std::cout << url << ", " << data_hash << ", " << license << std::endl;
      }
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
