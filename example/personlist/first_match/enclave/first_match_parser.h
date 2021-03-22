#include "stbox/stx_common.h"
#include "user_type.h"
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/query/filter.h>
#include <string.h>

class first_match_parser {
public:
  first_match_parser() {}
  first_match_parser(
      ::hpda::extractor::internal::extractor_base<user_item_t> *source)
      : m_source(source){};

  inline std::string do_parse(const std::string &param) {

    int counter = 0;
    hpda::processor::internal::filter_impl<user_item_t> match(
        m_source, [&](const user_item_t &v) {
          counter++;
          std::string target = param;
          std::string zjhm = v.get<ZJHM>();
          if (memcmp(zjhm.c_str(), target.c_str(), zjhm.size()) == 0) {
            return true;
          }
          return false;
        });

    hpda::output::internal::memory_output_impl<user_item_t> mo(&match);
    mo.run();

    std::string result = "";
    for (auto it : mo.values()) {
      stbox::printf("found\n");
      result += it.get<XM>();
      result += " : ";
      result += it.get<ZJHM>();
      result += " .";
    }
    stbox::printf("result: %s\n", result.c_str());
    stbox::printf("checked %d items\n", counter);
    return result;
  }

  inline bool merge_parse_result(const std::vector<std::string> &block_result,
                                 const std::string &param,
                                 std::string &result) {
    std::string s;
    for (auto k : block_result) {
      s = s + k;
    }
    result = s;
    return false;
  }

protected:
  hpda::extractor::internal::extractor_base<user_item_t> *m_source;
};

