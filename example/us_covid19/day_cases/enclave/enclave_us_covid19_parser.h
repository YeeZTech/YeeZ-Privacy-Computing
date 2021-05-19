
#include "stbox/ebyte.h"
#include "stbox/stx_common.h"
#include "user_type.h"
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>

class enclave_us_covid19_parser {
public:
  enclave_us_covid19_parser() {}
  enclave_us_covid19_parser(
      ::hpda::extractor::internal::extractor_base<user_item_t> *source)
      : m_source(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    stbox::bytes result;
    return result;
  }
  inline bool merge_parse_result(const std::vector<stbox::bytes> &block_result,
                                 const stbox::bytes &param,
                                 stbox::bytes &result) {
    return false;
  }

protected:
  hpda::extractor::internal::extractor_base<user_item_t> *m_source;
};
