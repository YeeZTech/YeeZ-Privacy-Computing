#include "eparser_t.h"
#include "libsvm.h"
#include "type.h"
#include "ypc/common/crypto_prefix.h"
#include "ypc/core_t/analyzer/analyzer_context.h"
#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/corecommon/crypto/stdeth.h"
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

using ecc = ypc::crypto::eth_sgx_crypto;
define_nt(shu_pkey, stbox::bytes);
define_nt(dian_pkey, stbox::bytes);
typedef ::ff::net::ntpackage<0x3b549098, shu_pkey, dian_pkey> pkeys_pkg_t;

class pred_parser {
public:
  pred_parser() {}
  pred_parser(std::vector<std::shared_ptr<ypc::data_source_with_dhash>> &source)
      : m_datasources(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    stbox::bytes result;
    auto pkg = ypc::make_package<pkeys_pkg_t>::from_bytes(param);
    auto shu_pkey = pkg.get<::shu_pkey>();
    auto dian_pkey = pkg.get<::dian_pkey>();
    stbox::bytes shu_skey;
    auto ret = m_ctx->request_private_key_for_public_key(shu_pkey, shu_skey,
                                                         dian_pkey);
    if (ret) {
      std::string err = "request_private_key_for_public_key failed!";
      LOG(ERROR) << "request_private_key_for_public_key failed, code: " << ret;
      result = stbox::bytes(err);
      return result;
    }

    LOG(INFO) << "do parse";
    if (m_datasources.size() != 2) {
      return stbox::bytes("Should include two data sources");
    }

    // set intersection
    hpda::processor::internal::ordered_intersection_impl<pred_merge_item_t,
                                                         std::string>
        oi;
    ypc::to_type<stbox::bytes, pred_a_item_t> conv_a(m_datasources[0].get());
    oi.add_upper_stream<::id>(&conv_a);
    ypc::to_type<stbox::bytes, pred_b_item_t> conv_b(m_datasources[1].get());
    oi.add_upper_stream<::id>(&conv_b);

    hpda::output::internal::memory_output_impl<pred_merge_item_t> mo(&oi);
    mo.get_engine()->run();
    LOG(INFO) << "do parse done";

    if (mo.values().empty()) {
      result = stbox::bytes("not found!");
    }
    LOG(INFO) << "pred mo.values size: " << mo.values().size();
    // for (auto &it : mo.values()) {
    // result += it.get<::id>();
    // result += ",";
    //}

    auto handle_c = [](std::vector<int> &c, int offset) {
      for (auto &ele : c) {
        ele = ele + offset;
      }
    };
    // convert to libsvm format
    LOG(INFO) << "construct libsvm";
    libsvm ls(mo.values());
    LOG(INFO) << "construct libsvm done";
    std::vector<int> ignore_c({10, 11, 14, 17, 19, 29, 30, 35, 38, 40});
    handle_c(ignore_c, -1);
    LOG(INFO) << "ignore_one_hot_for_some_columns";
    ls.ignore_one_hot_for_some_columns(ignore_c);
    LOG(INFO) << "ignore_one_hot_for_some_columns done";
    std::vector<int> one_hot_c(
        {8, 9, 12, 13, 15, 16, 18, 20, 28, 31, 32, 33, 34, 36, 37, 39});
    handle_c(one_hot_c, -1);
    LOG(INFO) << "convert_to_libsvm";
    ls.convert_to_libsvm(one_hot_c, -1);
    LOG(INFO) << "convert_to_libsvm done";
    // ls.show_raw_rows();
    // ls.show_libsvm_rows();
    const auto &rows = ls.get_libsvm_rows();
    const auto &ids = ls.get_ids();

    // load encrypted model
    std::string filename("train.model");
    uint32_t enc_model_size;
    stbox::ocall_cast<uint32_t>(ocall_get_model_size)(
        (uint8_t *)&filename[0], filename.size(), &enc_model_size);
    stbox::bytes enc_model(enc_model_size);
    uint32_t offset = 0;
    while (offset + ypc::utc::max_item_size < enc_model.size()) {
      stbox::ocall_cast<uint32_t>(ocall_load_model)(
          (uint8_t *)&filename[0], filename.size(), enc_model.data() + offset,
          ypc::utc::max_item_size, offset);
      offset += ypc::utc::max_item_size;
    }
    stbox::ocall_cast<uint32_t>(ocall_load_model)(
        (uint8_t *)&filename[0], filename.size(), enc_model.data() + offset,
        enc_model.size() - offset, offset);
    LOG(INFO) << "load encrypted model, size: " << enc_model_size;

    // decrypt model
    uint32_t model_size =
        ecc::get_decrypt_message_size_with_prefix(enc_model_size);
    stbox::bytes model(model_size);
    auto se_ret = (sgx_status_t)ecc::decrypt_message_with_prefix(
        shu_skey, enc_model, model, ::ypc::utc::crypto_prefix_arbitrary);
    if (se_ret) {
      std::string err = "decrypt_message_with_prefix forward returns ";
      err += stbox::status_string(se_ret);
      LOG(ERROR) << err;
      result = stbox::bytes(err);
      return result;
    }
    LOG(INFO) << "model size: " << model_size;

    // pred
    xgboost::regression::RegBoostTask pred("pred", rows, ids);
    pred.set_model(std::string((const char *)model.data(), model.size()));
    pred.run();
    LOG(INFO) << "pred done";

    const auto &preds = pred.get_preds();
    const auto &pred_ids = pred.get_pred_ids();
    std::string pred_result_s;
    for (int i = 0; i < preds.size(); i++) {
      if (i % 10000 == 0 ) {
        LOG(INFO) << "pred_result rows: " << i;
      }
      pred_result_s += pred_ids[i];
      pred_result_s += ",";
      if (preds[i] < 0.5f) {
        pred_result_s += "0\n";
      } else {
        pred_result_s += "1\n";
      }
    }
    LOG(INFO) << "pred result size: " << pred_result_s.size();
    stbox::bytes pred_result(pred_result_s);
    LOG(INFO) << "generate result done";
    // encrypt pred results
    stbox::bytes pkey;
    se_ret = (sgx_status_t)ecc::generate_pkey_from_skey(shu_skey, pkey);
    if (se_ret) {
      std::string err = "generate_pkey_from_skey returns ";
      err += stbox::status_string(se_ret);
      LOG(ERROR) << err;
      result = stbox::bytes(err);
      return result;
    }
    stbox::bytes enc_result;
    se_ret = (sgx_status_t)ecc::encrypt_message_with_prefix(
        pkey, pred_result, ypc::utc::crypto_prefix_arbitrary, enc_result);
    if (se_ret) {
      std::string err = "encrypt_message_with_prefix returns ";
      err += stbox::status_string(se_ret);
      LOG(ERROR) << err;
      result = stbox::bytes(err);
      return result;
    }
    LOG(INFO) << "encrypt result done";
    // dump pred results
    std::string filepred("pred.result");
    offset = 0;
    while (offset + ypc::utc::max_item_size < enc_result.size()) {
      stbox::ocall_cast<uint32_t>(ocall_dump_model)(
          (uint8_t *)&filepred[0], filepred.size(), enc_result.data() + offset,
          ypc::utc::max_item_size, offset);
      offset += ypc::utc::max_item_size;
    }
    stbox::ocall_cast<uint32_t>(ocall_dump_model)(
        (uint8_t *)&filepred[0], filepred.size(), enc_result.data() + offset,
        enc_result.size() - offset, offset);
    LOG(INFO) << "dump result done";
    std::string msg;
    msg += ("pred result nums: " + std::to_string(preds.size()));
    return stbox::bytes(msg);
  }

  void set_context(ypc::analyzer_context *ctx) { m_ctx = ctx; }

protected:
  std::vector<std::shared_ptr<ypc::data_source_with_dhash>> m_datasources;
  ypc::analyzer_context *m_ctx;
};
