#include "libsvm.h"
#include "type.h"
#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/corecommon/data_source.h"
#include "ypc/corecommon/package.h"
#include "ypc/corecommon/to_type.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#include "ypc/stbox/tsgx/log.h"

#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/query/filter.h>
#include <hpda/processor/set/intersection.h>
#include <hpda/processor/transform/concat.h>

#include "xgboost/regression/xgboost_reg_task.h"

class pred_parser {
public:
  pred_parser() {}
  pred_parser(std::vector<std::shared_ptr<ypc::data_source_with_dhash>> &source)
      : m_datasources(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    LOG(INFO) << "do parse";
    if (m_datasources.size() != 2) {
      return stbox::bytes("Should include two data sources");
    }

    // set intersection
    hpda::processor::internal::ordered_intersection_impl<train_merge_item_t,
                                                         std::string>
        oi;
    ypc::to_type<stbox::bytes, train_a_item_t> conv_a(m_datasources[0].get());
    oi.add_upper_stream<::id>(&conv_a);
    ypc::to_type<stbox::bytes, train_b_item_t> conv_b(m_datasources[1].get());
    oi.add_upper_stream<::id>(&conv_b);

    hpda::output::internal::memory_output_impl<train_merge_item_t> mo(&oi);
    mo.get_engine()->run();
    LOG(INFO) << "do parse done";

    stbox::bytes result;
    if (mo.values().empty()) {
      result = stbox::bytes("not found!");
    }
    LOG(INFO) << "mo.values size: " << mo.values().size();
    // for (auto &it : mo.values()) {
    // result += it.get<::id>();
    // result += ",";
    //}

    // convert to libsvm format
    libsvm ls(mo.values());
    ls.ignore_one_hot_for_some_columns(
        {10, 11, 14, 17, 19, 29, 30, 35, 38, 40});
    ls.convert_to_libsvm(
        {8, 9, 12, 13, 15, 16, 18, 20, 28, 31, 32, 33, 34, 36, 37, 39});
    const auto &rows = ls.get_libsvm_rows();
    const auto &ids = ls.get_ids();

    // train
    xgboost::regression::RegBoostTask train("train", rows, ids);
    train.run();
    const std::string &model = train.get_model();
    LOG(INFO) << "model size: " << model.size();
    // pred
    xgboost::regression::RegBoostTask pred("pred", rows, ids);
    pred.set_model(model);
    pred.run();

    const auto &preds = pred.get_preds();
    const auto &pred_ids = pred.get_pred_ids();
    for (int i = 0; i < std::min(size_t(100), preds.size()); i++) {
      result += pred_ids[i];
      result += ",";
      if (preds[i] < 0.5f) {
        result += "0\n";
      } else {
        result += "1\n";
      }
    }
    return result;
  }

protected:
  std::vector<std::shared_ptr<ypc::data_source_with_dhash>> m_datasources;
};
