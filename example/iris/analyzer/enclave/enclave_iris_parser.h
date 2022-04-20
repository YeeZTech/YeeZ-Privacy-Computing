#include "stbox/ebyte.h"
#include "stbox/stx_common.h"
#include "ypc_t/analyzer/data_source.h"
#ifdef YPC_SGX
#include "stbox/tsgx/log.h"
#else
#include <glog/logging.h>
#endif
#include "user_type.h"

#include "corecommon/to_type.h"
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/processor_base.h>
#include <hpda/processor/transform/concat.h>

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

typedef ff::util::ntobject<sepal_len, sepal_wid, petal_len, petal_wid, species>
    extra_nt_t;
class transform_format
    : public hpda::processor::processor_base_t<extra_nt_t, user_item_t> {
public:
  transform_format(::hpda::processor_with_output_t<extra_nt_t> *t)
      : hpda::processor::processor_base_t<extra_nt_t, user_item_t>(t) {}

  virtual bool process() {
    if (!has_input_value()) {
      return false;
    }

    auto t = base::input_value();
    m_data.get<iris_data>().set<sepal_len, sepal_wid, petal_len, petal_wid>(
        t.get<sepal_len>(), t.get<sepal_wid>(), t.get<petal_len>(),
        t.get<petal_wid>());
    m_data.set<species>(t.get<species>());
    base::consume_input_value();
    return true;
  }
  virtual user_item_t output_value() { return m_data; }

protected:
  user_item_t m_data;
};

class enclave_iris_parser {
public:
  enclave_iris_parser(ypc::data_source<stbox::bytes> *source)
      : m_source(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    ypc::to_type<stbox::bytes, extra_nt_t> converter(m_source);
    transform_format trans(&converter);

    hpda::algorithm::kmeans::kmeans_processor<
        hpda::ntobject<iris_data, species>, iris_data, double, iid>
        km(&trans, 3, 0.001);

    hpda::output::memory_output<iris_data, species, iid> mo(
        km.data_with_cluster_stream());

    mo.get_engine()->run();
    stbox::bytes result;
    int i = 0;
    for (auto it : mo.values()) {
      result += it.get<species>();
      result += " - ";
      result += std::to_string(it.get<iid>());
      result += "\n";
      i++;
      // LOG(INFO) << i << ": " << it.get<species>() << " - "
      //<< std::to_string(it.get<iid>());
    }
    return result;
  }

protected:
  ypc::data_source<stbox::bytes> *m_source;
};

class enclave_iris_means_parser {
public:
  enclave_iris_means_parser(ypc::data_source<stbox::bytes> *source)
      : m_source(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    ypc::to_type<stbox::bytes, extra_nt_t> converter(m_source);
    transform_format trans(&converter);

    typedef hpda::algorithm::kmeans::kmeans_processor<
        hpda::ntobject<iris_data, species>, iris_data, double, iid>
        kmeans_t;
    kmeans_t km(&trans, 3, 0.001);

    hpda::output::memory_output<iid, kmeans_t::mean_point,
                                kmeans_t::average_distance>
        mo(km.means_stream());

    mo.get_engine()->run();
    stbox::bytes result;
    int i = 0;
    for (auto it : mo.values()) {
      result += std::to_string(it.get<iid>());
      result += " - ";
      auto id = it.get<kmeans_t::mean_point>();
      result += " (" + std::to_string(id.template get<sepal_len>());
      result += " ," + std::to_string(id.get<sepal_wid>());
      result += " ," + std::to_string(id.get<petal_len>());
      result += " ," + std::to_string(id.get<petal_wid>());
      result += ") ";
      result += "\n";
      i++;
      // LOG(INFO) << i << ": " << it.get<species>() << " - "
      //<< std::to_string(it.get<iid>());
    }
    return result;
  }

protected:
  ypc::data_source<stbox::bytes> *m_source;
};
