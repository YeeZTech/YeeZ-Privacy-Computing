#include "../analyzer/enclave/enclave_iris_parser.h"
//#include "../analyzer/enclave/iris_parser.h"
#include "./privacy_data_stream.h"
#include "common/ignore.h"
#include "stbox/ebyte.h"
#include "toolkit/plugins/csv/csv_reader.h"
#include "ypc/block_data_source.h"
#include "ypc_t/analyzer/ntpackage_item_parser.h"
#include <iostream>

int main(int argc, char *argv[]) {

  hpda::engine e;
  // ypc::privacy_data_stream<user_item_t> pds(argv[1], argv[2],
  // parse_item_data);

  // clang-format off
  std::string json("{\"file_path\":\"../bin/iris.data\"}");
  // clang-format on
  ypc::plugins::typed_csv_reader<extra_nt_t> r("./iris.data");

  hpda::extractor::internal::raw_data_impl<extra_nt_t> raw;
  auto c = r.get_item_number();
  for (int i = 0; i < c; ++i) {
    auto t = r.read_typed_item();
    raw.add_data(t);
  }
  raw.set_engine(&e);

  transform_format tf(&raw);

  stbox::bytes param;

  enclave_iris_parser fmp(&tf, ypc::ignore);

  auto result = fmp.do_parse(param);
  std::cout << std::string((char *)result.data(), result.size()) << std::endl;
  return 0;
}
