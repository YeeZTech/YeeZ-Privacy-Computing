#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <hpda/algorithm/kmeans.h>
#include <hpda/extractor/paged_https_extractor.h>
#include <hpda/hpda.h>
#include <hpda/processor/transform/json_to_data_batch.h>
#include <iostream>

define_nt(sepal_len, double);
define_nt(sepal_wid, double);
define_nt(petal_len, double);
define_nt(petal_wid, double);
define_nt(species, std::string);
define_nt(iid, int);
typedef hpda::ntobject<sepal_len, sepal_wid, petal_len, petal_wid> iris_data_t;
define_nt(iris_data, iris_data_t);

typedef hpda::ntobject<iris_data, species> iris_item_t;

namespace hpda {
template <typename T1, typename T2> struct euclidean;

template <> struct euclidean<iris_data_t, double> {
  static double distance_square(const iris_data_t &p1, const iris_data_t &p2) {
    double ret = 0;
    auto v1 = p1.get<sepal_len>() - p2.get<sepal_len>();
    auto v2 = p1.get<sepal_wid>() - p2.get<sepal_wid>();
    auto v3 = p1.get<petal_len>() - p2.get<petal_len>();
    auto v4 = p1.get<petal_wid>() - p2.get<petal_wid>();

    ret += v1 * v1;
    ret += v2 * v2;
    ret += v3 * v3;
    ret += v4 * v4;
    return ret;
  }
};
} // namespace hpda

iris_data_t operator+(const iris_data_t &p1, const iris_data_t &p2) {
  iris_data_t d;
  d.set<sepal_len>(p1.get<sepal_len>() + p2.get<sepal_len>());
  d.set<sepal_wid>(p1.get<sepal_wid>() + p2.get<sepal_wid>());
  d.set<petal_len>(p1.get<petal_len>() + p2.get<petal_len>());
  d.set<petal_wid>(p1.get<petal_wid>() + p2.get<petal_wid>());
  return d;
}

iris_data_t operator/(const iris_data_t &p1, size_t v) {
  iris_data_t d;
  LOG(INFO) << "operator /" << v;
  d.set<sepal_len>(p1.get<sepal_len>() / v);
  d.set<sepal_wid>(p1.get<sepal_wid>() / v);
  d.set<petal_len>(p1.get<petal_len>() / v);
  d.set<petal_wid>(p1.get<petal_wid>() / v);
  return d;
}

iris_item_t km_convert_to_iris(const std::string &str) {
  std::vector<std::string> result;
  boost::split(result, str, boost::is_any_of(","));
  if (result.size() != 5) {
    throw std::runtime_error("invalid data");
  }
  iris_data_t d;
  d.set<sepal_len, sepal_wid>(std::stod(result[0]), std::stod(result[1]));
  d.set<petal_len, petal_wid>(std::stod(result[2]), std::stod(result[3]));
  iris_item_t ret;
  ret.set<species>(result[4]);
  ret.set<iris_data>(d);
  return ret;
}

// std::ostream &operator<<(std::ostream &os, const iris_item_t &obj) {
// os << "(" << obj.get<sepal_len>() << ", " << obj.get<sepal_wid>() << ", "
//<< obj.get<petal_len>() << ", " << obj.get<petal_wid>() << ", "
//<< obj.get<species>() << ")";
// return os;
//}

TEST(kmeans_iris, basic) {
  hpda::extractor::paged_https_extractor phe(
      "https://asresearch.io", "/iris-data?", 30,
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

  hpda::processor::json_to_data_batch<iris_data, species> trans(
      &phe,
      [](const boost::property_tree::ptree &ptree)
          -> const boost::property_tree::ptree & {
        LOG(INFO) << "t";
        return ptree.get_child("data");
      },
      km_convert_to_iris);

  // while (trans.next_output()) {
  // std::cout << trans.output_value() << std::endl;
  //}

  hpda::algorithm::kmeans::kmeans_processor<hpda::ntobject<iris_data, species>,
                                            iris_data, double, iid>
      km(&trans, 3, 0.001);

  hpda::output::memory_output<iris_data, species, iid> mo(
      km.data_with_cluster_stream());
  LOG(INFO) << "engine.run() start";
  engine.run();

  LOG(INFO) << "engine.run() end";
  for (auto it : mo.values()) {
    std::cout << it.get<species>() << ", " << it.get<iid>() << std::endl;
  }
}
