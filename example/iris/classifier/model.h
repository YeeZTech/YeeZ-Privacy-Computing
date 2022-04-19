#include "stbox/stx_common.h"
#ifdef YPC_SGX
#include "stbox/ebyte.h"
#include "stbox/tsgx/log.h"
#include "ypc_t/analyzer/data_source.h"
#include "ypc_t/analyzer/to_type.h"
#else
#include <glog/logging.h>
#endif

#include "user_type.h"

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
define_nt(cid, std::string);

typedef hpda::algorithm::kmeans::kmeans_processor<
    hpda::ntobject<iris_data, species>, iris_data, double, iid>
    kmeans_t;
typedef kmeans_t::average_distance average_distance_t;
typedef ff::util::ntobject<cid, iris_data, average_distance_t> mean_t;
define_nt(mean, std::vector<mean_t>);

typedef ff::util::ntobject<mean> means_t;
