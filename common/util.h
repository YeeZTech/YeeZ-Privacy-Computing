#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace ypc {
// Bytes can be stbox::bytes or ypc::bytes
template <typename Bytes> void change_endian(Bytes &pkey) {
  size_t step = pkey.size() / 2;
  for (size_t i = 0; i < pkey.size(); i += step) {
    for (size_t j = 0; j < step / 2; j++) {
      std::swap(pkey[i + j], pkey[i + step - 1 - j]);
    }
  }
}

template <typename T> void change_endian(T *pkey, size_t pkey_size) {
  static_assert(sizeof(T) == 1, "change_endian is only for 1 byte type");
  size_t step = pkey_size / 2;
  for (size_t i = 0; i < pkey_size; i += step) {
    for (size_t j = 0; j < step / 2; j++) {
      std::swap(pkey[i + j], pkey[i + step - 1 - j]);
    }
  }
}
} // namespace ypc
