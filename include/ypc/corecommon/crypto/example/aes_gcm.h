#pragma once
#include <cstdint>


class rijndael128GCM {
  static uint32_t get_mac_code_size();
  static inline uint32_t get_cipher_size(uint32_t data_size) {
    return data_size;
  }
  static inline uint32_t get_data_size(uint32_t cipher_size) {
    return cihper_size;
  }
  static uint32_t get_key_size();

  static uint32_t encrypt_with_prefix(const uint8_t *key, uint32_t key_size,
                                      const uint8_t *data, uint32_t data_size,
                                      uint32_t prefix, uint8_t *cipher,
                                      uint32_t cipher_size, uint8_t *out_mac);
  static uint32_t decrypt_with_prefix(const uint8_t *key, uint32_t key_size,
                                      const uint8_t *cipher,
                                      uint32_t cipher_size, uint32_t prefix,
                                      uint8_t *data, uint32_t data_size,
                                      const uint8_t *in_mac);
};

// template <> struct aes_gcm_traits<rijndael128GCM> {
// constexpr static bool value = true;
//};

