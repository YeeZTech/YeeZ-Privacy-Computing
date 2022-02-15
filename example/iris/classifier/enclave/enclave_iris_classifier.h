#include "../model.h"
#ifdef YPC_SGX
#include "stbox/tsgx/log.h"
#include "ypc_t/analyzer/to_type.h"
#else
#include <glog/logging.h>
#endif
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/processor_base.h>
#include <hpda/processor/transform/concat.h>



class enclave_iris_classifier {
public:
  enclave_iris_classifier(){};

  inline stbox::bytes do_parse(const means_t &means,
                               const stbox::bytes &param) {
    m_means = means.get<mean>();
    auto idata = ypc::make_package<iris_data_t>::from_bytes(param);
    double min = std::numeric_limits<double>::max();

    std::string class_id;
    for (int i = 0; i < m_means.size(); ++i) {
      auto d = hpda::euclidean<iris_data_t, double>::distance_square(
          idata, m_means[i].get<iris_data>());
      if (d < min) {
        class_id = m_means[i].get<cid>();
        min = d;
      }
    }
    stbox::bytes result;
    if (min == std::numeric_limits<double>::max()) {
      result += stbox::bytes("not found model");
    } else {
      result += stbox::bytes(class_id.data(), class_id.size());
      result += " - ";
      result += std::to_string(min);
      result += "\n";
    }

    LOG(INFO) << "result is: "
              << std::string((const char *)result.data(), result.size());
    return result;
  }

protected:
  std::vector<mean_t> m_means;
};
