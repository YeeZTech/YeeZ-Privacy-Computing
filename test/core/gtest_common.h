#pragma once

#include "stbox/eth/eth_hash.h"
#include "ypc/byte.h"
#include "ypc/memref.h"
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>

stbox::bytes random_string(size_t len);

template <typename FT>
stbox::bytes test_m_data(FT &f, const char *name, size_t item_num,
                         size_t item_len) {
  stbox::bytes data_hash = stbox::eth::keccak256_hash(stbox::bytes("test"));

  std::vector<stbox::bytes> write_items;
  std::vector<stbox::bytes> read_items;

  f.open_for_write(name);
  for (size_t i = 0; i < item_num; i++) {
    stbox::bytes item = random_string(item_len);
    write_items.push_back(item);
    data_hash = stbox::eth::keccak256_hash(data_hash + item);

    f.append_item(item.data(), item.size());
  }
  f.close();

  ypc::memref r;
  f.open_for_read(name);

  stbox::bytes kdata_hash = stbox::eth::keccak256_hash(stbox::bytes("test"));

  while (f.next_item(r)) {
    stbox::bytes item(r.data(), r.size());
    read_items.push_back(item);
    kdata_hash = stbox::eth::keccak256_hash(kdata_hash + item);
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
