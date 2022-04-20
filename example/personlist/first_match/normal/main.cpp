#include "../../common.h"
#include "common/ignore.h"
#include "stbox/ebyte.h"
#include "ypc/block_data_source.h"
#define EXAMPLE_FM_NORMAL
#include "../enclave/first_match_parser.h"
#undef EXAMPLE_FM_NORMAL

int main(int argc, char *argv[]) {

  std::string fp = "person_list";
  ypc::bytes param("421003198607263380");
  file_t ssf;
  ssf.open_for_read(fp.c_str());

  ypc::block_data_source<file_t> sds(&ssf);

  first_match_parser fmp(&sds);

  fmp.do_parse(param);
  return 0;
}
