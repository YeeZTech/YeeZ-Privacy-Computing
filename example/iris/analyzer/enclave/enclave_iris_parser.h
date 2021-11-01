#include "stbox/ebyte.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/log.h"
#include "user_type.h"
#include "ypc_t/analyzer/parser_wrapper_base.h"
#include "ypc_t/analyzer/to_type.h"

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
    : public hpda::processor::internal::processor_base<extra_nt_t,
                                                       user_item_t> {
public:
  transform_format(::hpda::internal::processor_with_output<extra_nt_t> *t)
      : hpda::processor::internal::processor_base<extra_nt_t, user_item_t>(t) {}

  virtual bool process() {
#ifdef YPC_SGX
    LOG(INFO) << "transformat process ";
#endif
    if (!has_input_value()) {
      return false;
    }

    auto t = base::input_value();
    m_data.get<iris_data>().set<sepal_len, sepal_wid, petal_len, petal_wid>(
        t.get<sepal_len>(), t.get<sepal_wid>(), t.get<petal_len>(),
        t.get<petal_wid>());
    m_data.set<species>(t.get<species>());
    return true;
  }
  virtual user_item_t output_value() { return m_data; }

protected:
  user_item_t m_data;
};

class enclave_iris_parser {
public:
  enclave_iris_parser() {}
  enclave_iris_parser(
      ::hpda::extractor::internal::extractor_base<user_item_t> *source,
      const std::vector<ypc::extra_data_source_group> &extra_data_sources)
      : m_source(source), m_extra_data_source(extra_data_sources){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    hpda::processor::concat<iris_data, species> concat(m_source);
    std::vector<std::shared_ptr<ypc::to_type<extra_nt_t>>> m_tts;
    std::vector<std::shared_ptr<transform_format>> tfs;

    LOG(INFO) << "1, size: " << m_extra_data_source.size();
    for (auto it : m_extra_data_source) {
      LOG(INFO) << "1.1, size: " << it.data_sources.size();
      for (auto ds : it.data_sources) {
        m_tts.push_back(std::make_shared<ypc::to_type<extra_nt_t>>(ds.get()));
        tfs.push_back(
            std::make_shared<transform_format>(m_tts[m_tts.size() - 1].get()));
        concat.add_upper_stream(tfs[tfs.size() - 1].get());
      }
    }
    LOG(INFO) << "2, tfs size: " << tfs.size();

    hpda::algorithm::kmeans::kmeans_processor<
        hpda::ntobject<iris_data, species>, iris_data, double, iid>
        km(&concat, 3, 0.001);

    LOG(INFO) << "3";
    hpda::output::memory_output<iris_data, species, iid> mo(
        km.data_with_cluster_stream());

    LOG(INFO) << "4";
    mo.run();
    LOG(INFO) << "5";
    stbox::bytes result;
    for (auto it : mo.values()) {
      result += it.get<species>();
      result += " - ";
      result += std::to_string(it.get<iid>());
      result += "\n";
    }
    LOG(INFO) << "6";
    return result;
  }
  inline bool merge_parse_result(const std::vector<stbox::bytes> &block_result,
                                 const stbox::bytes &param,
                                 stbox::bytes &result) {
    return false;
  }

protected:
  hpda::extractor::internal::extractor_base<user_item_t> *m_source;
  std::vector<ypc::extra_data_source_group> m_extra_data_source;
};
