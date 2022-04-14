#include "sgx_plugin.h"
#include "ypc_t/analyzer/ntpackage_item_parser.h"
#include <cstdint>
#include <stdexcept>

uint64_t ypc_plugin_version() { return 1; }


namespace ypc {
user_item_t ecall_parse_item_data(const uint8_t *data, size_t len) {

  typedef ff::util::ntobject<sepal_len, sepal_wid, petal_len, petal_wid,
                             species>
      utt;

  // iris reader plugin should be generated in plugin toolkit
  utt rt = ntpackage_item_parser<uint8_t, utt>::parser(data, len);

  user_item_t ret;
  ret.get<iris_data>().set<sepal_len, sepal_wid, petal_len, petal_wid>(
      rt.template get<sepal_len>(), rt.template get<sepal_wid>(),
      rt.template get<petal_len>(), rt.template get<petal_wid>());
  ret.set<species>(rt.template get<species>());
  return ret;
}
} // namespace ypc
