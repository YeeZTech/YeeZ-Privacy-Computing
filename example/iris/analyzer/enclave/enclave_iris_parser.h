#include "stbox/ebyte.h"
#include "stbox/stx_common.h"
#include "user_type.h"

#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>

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
  d.set<sepal_len>(p1.get<sepal_len>() / v);
  d.set<sepal_wid>(p1.get<sepal_wid>() / v);
  d.set<petal_len>(p1.get<petal_len>() / v);
  d.set<petal_wid>(p1.get<petal_wid>() / v);
  return d;
}
#include <hpda/algorithm/kmeans.h>
define_nt(iid, int);

class enclave_iris_parser {
public:
  enclave_iris_parser() {}
  template <typename ET>
  enclave_iris_parser(
      ::hpda::extractor::internal::extractor_base<user_item_t> *source,
      ET &&ignore)
      : m_source(source) {
  };

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    hpda::algorithm::kmeans::kmeans_processor<
        hpda::ntobject<iris_data, species>, iris_data, double, iid>
        km(m_source, 3, 0.001);

    hpda::output::memory_output<iris_data, species, iid> mo(
        km.data_with_cluster_stream());
    mo.run();
    stbox::bytes result;
    for (auto it : mo.values()) {
      result += it.get<species>();
      result += " - ";
      result += std::to_string(it.get<iid>());
      result += "\n";
    }
    return result;
  }
  inline bool merge_parse_result(const std::vector<stbox::bytes> &block_result,
                                 const stbox::bytes &param,
                                 stbox::bytes &result) {
    return false;
  }

protected:
  hpda::extractor::internal::extractor_base<user_item_t> *m_source;
};
