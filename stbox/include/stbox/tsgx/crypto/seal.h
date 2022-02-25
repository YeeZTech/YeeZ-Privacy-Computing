#pragma once
#include "stbox/ebyte.h"

namespace stbox {
namespace crypto {
template <typename Device> struct raw_device_sealer {

  static uint32_t get_sealed_data_size(uint32_t data_size);

  static uint32_t seal_data(const uint8_t *data, uint32_t data_size,
                            uint8_t *sealed_data, uint32_t sealed_size);

  static uint32_t get_unsealed_data_size(const uint8_t *sealed_data,
                                         uint32_t sealed_data_size);

  static uint32_t unseal_data(const uint8_t *sealed_data,
                              uint32_t sealed_data_size, uint8_t *data,
                              uint32_t data_size);

};

template <typename Device> struct device_sealer {
  typedef raw_device_sealer<Device> raw_t;

  static uint32_t get_sealed_data_size(uint32_t data_size) {
    return raw_t::get_sealed_data_size(data_size);
  }
  static uint32_t seal_data(const bytes &data, bytes &sealed_data) {
    sealed_data = bytes(get_sealed_data_size(data.size()));
    return raw_t::seal_data(data.data(), data.size(), sealed_data.data(),
                            sealed_data.size());
  }
  static uint32_t get_unsealed_data_size(const bytes &sealed_data) {
    return raw_t::get_unsealed_data_size(sealed_data.data(),
                                         sealed_data.size());
  }

  static uint32_t unseal_data(const bytes &sealed_data, bytes &data) {
    return raw_t::unseal_data(sealed_data.data(), sealed_data.size(),
                              data.data(), data.size());
  }
};
} // namespace crypto
} // namespace stbox
