#include "stbox/ebyte.h"
#include "stbox/stx_common.h"
#ifdef EXAMPLE_FM_NORMAL
#include "glog/logging.h"
#else
#include "stbox/tsgx/log.h"
#endif
#include "user_type.h"
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/query/filter.h>
#include <string.h>

class first_match_parser {
public:
  first_match_parser() {}
  template <typename ET>
  first_match_parser(
      ::hpda::extractor::internal::extractor_base<user_item_t> *source,
      ET &&ignore)
      : m_source(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    LOG(INFO) << "do parse";
    int counter = 0;
    hpda::processor::internal::filter_impl<user_item_t> match(
        m_source, [&](const user_item_t &v) {
          counter++;
          std::string zjhm = v.get<ZJHM>();
          if (memcmp(zjhm.c_str(), param.data(), zjhm.size()) == 0) {
            return true;
          }
          return false;
        });

    hpda::output::internal::memory_output_impl<user_item_t> mo(&match);
    mo.get_engine()->run();
    LOG(INFO) << "do parse done";

    stbox::bytes result;
    for (auto it : mo.values()) {
      stbox::printf("found\n");
      result += it.get<XM>();
      result += " : ";
      result += it.get<ZJHM>();
      result += " .";
    }
    return result;
  }

  inline bool merge_parse_result(const std::vector<stbox::bytes> &block_result,
                                 const stbox::bytes &param,
                                 stbox::bytes &result) {
    stbox::bytes s;
    for (auto k : block_result) {
      s = s + k;
    }
    result = s;
    return false;
  }

protected:
  hpda::extractor::internal::extractor_base<user_item_t> *m_source;
};

