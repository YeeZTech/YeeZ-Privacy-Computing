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

stbox::bytes random_string(size_t len) {
  std::string ret(len, '0');

  for (size_t i = 0; i < len; i++) {
    ret[i] = int('a') + i % 26;
  }
  return stbox::bytes(ret.data(), ret.size());
}

template <typename FT>
stbox::bytes test_m_data(FT &f, const char *name, size_t item_num,
                         size_t item_len) {
  stbox::bytes data_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(stbox::bytes("test"), data_hash);

  std::vector<stbox::bytes> write_items;
  std::vector<stbox::bytes> read_items;

  f.open_for_write(name);
  for (size_t i = 0; i < item_num; i++) {
    stbox::bytes item = random_string(item_len);
    write_items.push_back(item);
    ypc::crypto::eth_sgx_crypto::hash_256(data_hash + item, data_hash);

    f.append_item(item.data(), item.size());
  }
  f.close();

  f.open_for_read(name);

  stbox::bytes kdata_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(stbox::bytes("test"), kdata_hash);

  std::unique_ptr<char[]> buf(new char[FT::BlockSizeLimit]);
  size_t buf_size;
  while (f.next_item(buf.get(), FT::BlockSizeLimit, buf_size) == FT::succ) {
    stbox::bytes item(buf.get(), buf_size);
    read_items.push_back(item);
    ypc::crypto::eth_sgx_crypto::hash_256(kdata_hash + item, kdata_hash);
  }
  f.close();

  if (data_hash != kdata_hash) {
    // std::cout << "write " << write_items.size() << " items, read "
    //<< read_items.size() << " items!" << std::endl;
    for (size_t i = 0; i < write_items.size(); i++) {
      if (write_items[i] != read_items[i]) {
        // std::cout << "diff i:" << i << ", " << write_items[i] << "  --->   "
        //<< read_items[i] << std::endl;
      }
    }
  }

  EXPECT_TRUE(data_hash == kdata_hash);
  return data_hash;
}

typedef ypc::blockfile<0x29384792, 16, 1024> bft;

class hello {
public:
  inline stbox::bytes do_parse(const stbox::bytes &param) {
    // test many larget data
    bft f3;
    auto hash = test_m_data(f3, "tsf_in_sgx", 150, 1023);
    return hash;
  }
};

using Crypto = ypc::crypto::eth_sgx_crypto;

ypc::algo_wrapper<Crypto, ypc::noinput_data_stream, hello,

                  ypc::local_result>
    pw;

YPC_PARSER_IMPL(pw);
