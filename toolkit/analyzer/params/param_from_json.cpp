#include "params/param_from_json.h"
#include "ypc/byte.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

param_from_json::param_from_json(const std::string &path) : m_path(path) {}

void param_from_json::read_from_source() {
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(m_path, pt);
    m_eskey = ypc::bytes::from_hex(pt.get<std::string>("encrypted-skey"));
    m_input = ypc::bytes::from_hex(pt.get<std::string>("encrypted-input"));

    m_epkey = ypc::bytes::from_hex(pt.get<std::string>("provider-pkey"));
    m_ehash = ypc::bytes::from_hex(pt.get<std::string>("program-enclave-hash"));
    m_vpkey = ypc::bytes::from_hex(pt.get<std::string>("analyzer-pkey"));
    m_sig = ypc::bytes::from_hex(pt.get<std::string>("forward-sig"));
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format("Read forward params from file failed! Error: %1%") %
        e.what()));
  }
}

