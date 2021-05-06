#include "params/param_from_json.h"
#include "ypc/byte.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

param_from_json::param_from_json(const std::string &path) : m_path(path) {}

uint32_t param_from_json::read_from_source() {
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(m_path, pt);
    m_eskey = pt.get<ypc::bytes>("encrypted-skey");
    m_input = pt.get<ypc::bytes>("encrypted-input");

    m_epkey = pt.get<ypc::bytes>("provider-pkey");
    m_ehash = pt.get<ypc::bytes>("program-enclave-hash");
    m_vpkey = pt.get<ypc::bytes>("analyzer-pkey");
    m_sig = pt.get<ypc::bytes>("forward-sig");
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format("Read forward params from file failed! Error: %1%") %
        e.what()));
    return ::ypc::param_from_json_read_param_from_source_failed;
  }
  return ::ypc::success;
}

