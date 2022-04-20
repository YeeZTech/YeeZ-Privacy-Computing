#include "corecommon/package.h"
#include "stbox/ebyte.h"
#include "stbox/stx_common.h"
#ifdef EXAMPLE_FM_NORMAL
#include "glog/logging.h"
typedef ypc::bytes bytes;
#else
#include "stbox/tsgx/log.h"
#include "ypc_t/analyzer/data_source.h"
typedef stbox::bytes bytes;
#endif
#include "corecommon/data_source.h"
#include "corecommon/to_type.h"
#include "user_type.h"
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/query/filter.h>
#include <string.h>

define_nt(input_buf, std::string);
typedef ff::net::ntpackage<0, input_buf> input_buf_t;

class first_match_parser {
public:
  first_match_parser() {}
  first_match_parser(ypc::data_source<bytes> *source) : m_source(source){};

  inline bytes do_parse(const bytes &param) {
    LOG(INFO) << "do parse";
    ypc::to_type<bytes, user_item_t> converter(m_source);
    auto pkg = ypc::make_package<input_buf_t>::from_bytes(param);
    int counter = 0;
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

