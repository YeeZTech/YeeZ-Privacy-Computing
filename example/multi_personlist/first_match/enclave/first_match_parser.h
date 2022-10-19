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
#include <hpda/processor/transform/concat.h>
#include <string.h>

define_nt(input_buf, std::string);
typedef ff::net::ntpackage<0, input_buf> input_buf_t;

class first_match_parser {
public:
  first_match_parser() {}
  first_match_parser(
      std::vector<std::shared_ptr<ypc::data_source_with_dhash>> &source)
      : m_datasources(source){};

  inline bytes do_parse(const bytes &param) {
    LOG(INFO) << "do parse";
    auto pkg = ypc::make_package<input_buf_t>::from_bytes(param);
    int counter = 0;
    if (m_datasources.size() == 0) {
      return stbox::bytes("no data source");
    }

    typedef ypc::nt<stbox::bytes> ntt;
    hpda::processor::concat<ntt::data> concator(m_datasources[0].get());
    for (size_t i = 1; i < m_datasources.size(); i++) {
      concator.add_upper_stream(m_datasources[i].get());
    }

    ypc::to_type<bytes, user_item_t> converter(&concator);
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
  std::vector<std::shared_ptr<ypc::data_source_with_dhash>> m_datasources;
};

