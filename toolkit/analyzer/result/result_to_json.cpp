#include "result/result_to_json.h"
#include "ypc/byte.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

result_to_json::result_to_json(const std::string &path) : m_path(path) {}

void result_to_json::write_to_target(const result_pkg_t &res) {
  try {
    boost::property_tree::ptree pt;
    using ntt = ypc::nt<ypc::bytes>;
    pt.put("encrypted-result", res.get<ntt::encrypted_result>());
    pt.put("result-signature", res.get<ntt::result_signature>());
    pt.put("cost-signature", res.get<ntt::cost_signature>());
    pt.put("data-hash", res.get<ntt::data_hash>());
    boost::property_tree::json_parser::write_json(m_path, pt);
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format(
            "Write result and signature to target file failed! Error: %1%") %
        e.what()));
  }
}

void result_to_json::read_from_target(ypc::bytes &encrypted_result,
                                      ypc::bytes &result_signature,
                                      ypc::bytes &cost_signature,
                                      ypc::bytes &data_hash) {
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(m_path, pt);
    encrypted_result = pt.get<ypc::bytes>("encrypted-result");
    result_signature = pt.get<ypc::bytes>("result-signature");
    cost_signature = pt.get<ypc::bytes>("cost-signature");
    data_hash = pt.get<ypc::bytes>("data-hash");
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format(
            "Write result and signature to target file failed! Error: %1%") %
        e.what()));
  }
}
