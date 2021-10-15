#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace ypc {
namespace utc {

template <typename Bytes> void change_pubkey_endian(Bytes &pkey) {
  size_t step = pkey.size() / 2;
  for (size_t i = 0; i < pkey.size(); i += step) {
    for (size_t j = 0; j < step / 2; j++) {
      std::swap(pkey[i + j], pkey[i + step - 1 - j]);
    }
  }
}

template <typename T> void change_pubkey_endian(T *pkey, size_t pkey_size) {
  static_assert(sizeof(T) == 1, "change_endian is only for 1 byte type");
  size_t step = pkey_size / 2;
  for (size_t i = 0; i < pkey_size; i += step) {
    for (size_t j = 0; j < step / 2; j++) {
      std::swap(pkey[i + j], pkey[i + step - 1 - j]);
    }
  }
}

template <typename T> void endian_swap(T *pData, int start_index, int length) {
  static_assert(sizeof(T) == 1, "endian_swap is only for 1 byte type");
  int i, cnt, end, start;
  cnt = length / 2;
  start = start_index;
  end = start_index + length - 1;
  T tmp;
  for (i = 0; i < cnt; i++) {
    tmp = pData[start + i];
    pData[start + i] = pData[end - i];
    pData[end - i] = tmp;
  }
}
template <typename Bytes> void endian_swap(Bytes &data) {
  endian_swap(data.data(), 0, data.size());
}
} // namespace utc
} // namespace ypc
