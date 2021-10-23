#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <hpda/extractor/paged_https_extractor.h>
#include <hpda/processor/transform/json_to_data_batch.h>
#include <iostream>

define_nt(sepal_len, double);
define_nt(sepal_wid, double);
define_nt(petal_len, double);
define_nt(petal_wid, double);
define_nt(species, std::string);

typedef hpda::ntobject<sepal_len, sepal_wid, petal_len, petal_wid, species>
    iris_item_t;

iris_item_t convert_to_iris(const std::string &str) {
  std::vector<std::string> result;
  boost::split(result, str, boost::is_any_of(","));
  if (result.size() != 5) {
    throw std::runtime_error("invalid data");
  }
  iris_item_t ret;
  ret.set<sepal_len, sepal_wid>(std::stod(result[0]), std::stod(result[1]));
  ret.set<petal_len, petal_wid>(std::stod(result[2]), std::stod(result[3]));
  ret.set<species>(result[4]);
  return ret;
}

std::ostream &operator<<(std::ostream &os, const iris_item_t &obj) {
  os << "(" << obj.get<sepal_len>() << ", " << obj.get<sepal_wid>() << ", "
     << obj.get<petal_len>() << ", " << obj.get<petal_wid>() << ", "
     << obj.get<species>() << ")";
  return os;
}

TEST(json_to_data_batch, basic) {
  hpda::extractor::paged_https_extractor phe(
      "https://asresearch.io", "/iris-data?", 85,
      [](const std::string &prefix, int next_index, int page_limit) {
        std::stringstream ss;
        ss << prefix << "index=" << next_index << "&limit=" << page_limit;
        return ss.str();
      },
      [](const boost::property_tree::ptree &tree) {
        return tree.get<int>("start");
      },
      [](const boost::property_tree::ptree &tree) {
        return tree.get<int>("end");
      });

  hpda::engine engine;
  phe.set_engine(&engine);

  hpda::processor::json_to_data_batch<sepal_len, sepal_wid, petal_len,
                                      petal_wid, species>
  trans(
      &phe,
      [](const boost::property_tree::ptree &ptree)
          -> const boost::property_tree::ptree & {
        return ptree.get_child("data");
      },
      convert_to_iris);

  std::cout << "start data" << std::endl;
  while (trans.process()) {
    std::cout << trans.output_value() << std::endl;
  }
  std::cout << "end data" << std::endl;
}
