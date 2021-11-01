#include "../analyzer/enclave/enclave_iris_parser.h"
#include "../analyzer/enclave/iris_parser.h"
#include "./privacy_data_stream.h"
#include "common/ignore.h"
#include "stbox/ebyte.h"
#include "ypc/block_data_source.h"
#include <iostream>

int main(int argc, char *argv[]) {

  hpda::engine e;
  ypc::privacy_data_stream<user_item_t> pds(argv[1], argv[2], parse_item_data);
  pds.set_engine(&e);
  stbox::bytes param;

  enclave_iris_parser fmp(&pds, ypc::ignore);

  auto result = fmp.do_parse(param);
  std::cout << result << std::endl;
  return 0;
}
