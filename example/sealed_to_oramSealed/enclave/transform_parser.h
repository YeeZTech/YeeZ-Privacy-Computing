#include "ypc/corecommon/package.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#ifdef EXAMPLE_FM_NORMAL
#include <glog/logging.h>
typedef ypc::bytes bytes;
#else
#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/stbox/tsgx/log.h"
typedef stbox::bytes bytes;
#endif
#include "user_type.h"
#include "ypc/corecommon/data_source.h"
#include "ypc/corecommon/to_type.h"
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/query/filter.h>
#include <string.h>

define_nt(input_buf, std::string);
typedef ff::net::ntpackage<0, input_buf> input_buf_t;

class transform_parser {
public:
  transform_parser() {}
  transform_parser(ypc::data_source<bytes> *source) : m_source(source){};

  inline bytes do_parse(const bytes &param) {
    LOG(INFO) << "do parse";
    ypc::to_type<bytes, user_item_t> converter(m_source);
    // param must be serialized ntpackage
    auto pkg = ypc::make_package<input_buf_t>::from_bytes(param);
    int counter = 0;
    
    // 一个获取batch数量的OCALL   batch_num

    // 确定ORAM树的大小，真实块开始的位置real_index
    
    // build id map
    // 如何获得想要的索引项？，例如ZJHM

    // 需要在
    uint32_t batch_id = 1;
    std::vector<oram_ntt::id_map_pair> id_map_array;
    hpda::processor::internal::filter_impl<user_item_t> match(
        &converter, [&](const user_item_t &v) {
          // counter++;
          std::string item_index_field = v.get<ZJHM>();
          input_buf_t item_index_field_pkg;
          item_index_field_pkg.set<input_buf>(item_index_field);

          bytes item_index_field_bytes = ypc::make_bytes<bytes>::for_package(item_index_field_pkg);
          bytes item_index_field_hash;
          // TODO:需要获取加密方法
          ecc::hash_256(item_index_field_bytes, item_index_field_hash);


          return true;
        });

    hpda::processor::internal::filter_impl<user_item_t> match(
        &converter, [&](const user_item_t &v) {
          counter++;
          std::string zjhm = v.get<ZJHM>();
          if (zjhm == pkg.get<input_buf>()) {
            return true;
          }
          return false;
        });

    hpda::output::internal::memory_output_impl<user_item_t> mo(&match);
    mo.get_engine()->run();
    LOG(INFO) << "do parse done";

    bytes result;
    for (auto it : mo.values()) {
      stbox::printf("found\n");
      result += it.get<XM>();
      result += " : ";
      result += it.get<ZJHM>();
      result += " .";
    }
    return result;
  }

  inline bool merge_parse_result(const std::vector<bytes> &block_result,
                                 const bytes &param, bytes &result) {
    bytes s;
    for (auto k : block_result) {
      s = s + k;
    }
    result = s;
    return false;
  }

protected:
  ypc::data_source<bytes> *m_source;
};
