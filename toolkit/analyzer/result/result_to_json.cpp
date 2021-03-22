#include "result/result_to_json.h"
#include "ypc/byte.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

result_to_json::result_to_json(const std::string &path) : m_path(path) {}

void result_to_json::write_to_target(const ypc::bref &encrypted_result,
                                     const ypc::bref &result_signature,
                                     const ypc::bref &data_hash) {
  try {
    boost::property_tree::ptree pt;
    pt.put("encrypted-result", ypc::to_hex(encrypted_result));
    pt.put("result-signature", ypc::to_hex(result_signature));
    pt.put("data-hash", ypc::to_hex(data_hash));
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
                                      ypc::bytes &data_hash) {
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(m_path, pt);
    encrypted_result =
        ypc::bytes::from_hex(pt.get<std::string>("encrypted-result"));
    result_signature =
        ypc::bytes::from_hex(pt.get<std::string>("result-signature"));
    data_hash = ypc::bytes::from_hex(pt.get<std::string>("data-hash"));
  } catch (const std::exception &e) {
    throw std::runtime_error(boost::str(
        boost::format(
            "Write result and signature to target file failed! Error: %1%") %
        e.what()));
  }
}
