#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/interface/allowance_interface.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"

#include "ypc/core_t/file/blockfile.h"
#include "ypc/corecommon/to_type.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#include "ypc/stbox/tsgx/log.h"

#define EXPECT_EQ(a, b)                                                        \
  if ((a) != (b)) {                                                            \
    LOG(ERROR) << "test " << #a << #b << " failed";                            \
    throw std::runtime_error("test failed");                                   \
  }

#define EXPECT_TRUE(a)                                                         \
  if (!(a)) {                                                                  \
    LOG(ERROR) << "test " << #a << " failed";                                  \
    throw std::runtime_error("test failed");                                   \
  }

template <typename FT> stbox::bytes read_data(FT &f, const char *name) {

  f.open_for_read(name);
  if (!f.file().good()) {
    LOG(ERROR) << " cannot open file " << name;
  }

  stbox::bytes kdata_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(stbox::bytes("test"), kdata_hash);

  std::unique_ptr<char[]> buf(new char[FT::BlockSizeLimit]);
  size_t buf_size;
  size_t i = 0;
  while (f.next_item(buf.get(), FT::BlockSizeLimit, buf_size) == FT::succ) {
    stbox::bytes item(buf.get(), buf_size);
    ypc::crypto::eth_sgx_crypto::hash_256(kdata_hash + item, kdata_hash);
    i++;
  }
  f.close();

  return kdata_hash;
}

typedef ypc::blockfile<0x29384792, 16, 1024> bft;

class hello {
public:
  inline stbox::bytes do_parse(const stbox::bytes &param) {
    // test many small data
    bft f1;
    auto hash = read_data(f1, "tsf1_out");
    EXPECT_EQ(param, hash);
    return stbox::bytes("hello!");
  }
};

using Crypto = ypc::crypto::eth_sgx_crypto;
// using Crypto = ypc::crypto::gmssl_sgx_crypto;

ypc::algo_wrapper<Crypto, ypc::noinput_data_stream, hello,

                  ypc::local_result>
    pw;

YPC_PARSER_IMPL(pw);
