#pragma once
#include "ypc/byte.h"
#include "zallocator.h"

namespace ypc {
namespace openssl {
class sgx {
public:
  static int rijndael128_cmac_msg(const uint8_t *cmac_128bit_key,
                                  const uint8_t *p_src, uint32_t src_len,
                                  uint8_t *mac_128bit);

  static int rijndael128GCM_encrypt(const uint8_t *aes_gcm_128bit_key,
                                    const uint8_t *p_src, uint32_t src_len,
                                    uint8_t *p_dst, const uint8_t *p_iv,
                                    uint32_t iv_len, const uint8_t *p_add,
                                    uint32_t aad_len,
                                    uint8_t *aes_gcm_128_bit_out_mac);

  static int rijndael128GCM_decrypt(const uint8_t *aes_gcm_128bit_key,
                                    const uint8_t *p_src, uint32_t src_len,
                                    uint8_t *p_dst, const uint8_t *p_iv,
                                    uint32_t iv_len, const uint8_t *p_add,
                                    uint32_t aad_len,
                                    const uint8_t *aes_gcm_128bit_in_mac);

  static int sha256_msg(const uint8_t *message, size_t size,
                        uint8_t *sha256_hash);
};
} // namespace openssl
} // namespace ypc

