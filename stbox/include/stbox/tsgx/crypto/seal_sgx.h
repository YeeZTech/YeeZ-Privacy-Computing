#pragma once
#include "stbox/ebyte.h"
#include "stbox/tsgx/crypto/seal.h"
namespace stbox {
namespace crypto {
struct intel_sgx {};

template <> struct raw_device_sealer<intel_sgx> {

  static uint32_t get_sealed_data_size(uint32_t data_size);

  static uint32_t seal_data(const uint8_t *data, uint32_t data_size,
                            uint8_t *sealed_data, uint32_t sealed_size);

  static uint32_t get_unsealed_data_size(const uint8_t *sealed_data,
                                         uint32_t sealed_data_size);

  static uint32_t unseal_data(const uint8_t *sealed_data,
                              uint32_t sealed_data_size, uint8_t *data,
                              uint32_t data_size);

};
} // namespace crypto
} // namespace stbox
