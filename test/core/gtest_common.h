#pragma once

#include "ypc/core/byte.h"
#include "ypc/core/memref.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>

ypc::bytes random_string(size_t len);

template <typename FT>
ypc::bytes test_m_data(FT &f, const char *name, size_t item_num,
                       size_t item_len) {
  ypc::bytes data_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(ypc::bytes("test"), data_hash);

  std::vector<ypc::bytes> write_items;
  std::vector<ypc::bytes> read_items;

  f.open_for_write(name);
  for (size_t i = 0; i < item_num; i++) {
    ypc::bytes item = random_string(item_len);
    write_items.push_back(item);
    ypc::crypto::eth_sgx_crypto::hash_256(data_hash + item, data_hash);

    f.append_item(item.data(), item.size());
  }
  f.close();

  ypc::memref r;
  f.open_for_read(name);

  ypc::bytes kdata_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(ypc::bytes("test"), kdata_hash);

  while (f.next_item(r)) {
    ypc::bytes item(r.data(), r.size());
    read_items.push_back(item);
    ypc::crypto::eth_sgx_crypto::hash_256(kdata_hash + item, kdata_hash);
    r.dealloc();
  }
  f.close();

  if (data_hash != kdata_hash) {
    std::cout << "write " << write_items.size() << " items, read "
              << read_items.size() << " items!" << std::endl;
    for (size_t i = 0; i < write_items.size(); i++) {
      if (write_items[i] != read_items[i]) {
        std::cout << "diff i:" << i << ", " << write_items[i] << "  --->   "
                  << read_items[i] << std::endl;
      }
    }
  }

  EXPECT_TRUE(data_hash == kdata_hash);
  return data_hash;
}
