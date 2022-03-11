/*
 * Copyright (C) 2011-2020 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "sgx_ecp_types.h"
#include "stbox/ebyte.h"
#include "stbox/tsgx/crypto/ecp_interface.h"
#include "stbox/tsgx/log.h"
#include "stdlib.h"
#include "string.h"

#define MAC_KEY_SIZE 16
static uint8_t cmac_key[MAC_KEY_SIZE] = "yeez.tech.stbox";
#define EC_DERIVATION_BUFFER_SIZE(label_length) ((label_length) + 4)

#ifndef INTERNAL_SGX_ERROR_CODE_CONVERTOR
#define INTERNAL_SGX_ERROR_CODE_CONVERTOR(x)                                   \
  if (x != SGX_ERROR_OUT_OF_MEMORY) {                                          \
    x = SGX_ERROR_UNEXPECTED;                                                  \
  }
#endif

namespace stbox {
namespace crypto {

sgx_status_t derive_key(const sgx_ec256_dh_shared_t *shared_key,
                        const char *label, uint32_t label_length,
                        sgx_ec_key_128bit_t *derived_key) {
  sgx_status_t se_ret = SGX_SUCCESS;
  sgx_ec_key_128bit_t key_derive_key;
  if (!shared_key || !derived_key || !label) {
    return SGX_ERROR_INVALID_PARAMETER;
  }

  /*check integer overflow */
  if (label_length > EC_DERIVATION_BUFFER_SIZE(label_length)) {
    return SGX_ERROR_INVALID_PARAMETER;
  }

  LOG(INFO) << "ecdh key: " << stbox::bytes((const char *)shared_key, 32);

  se_ret = sgx_rijndael128_cmac_msg(
      (sgx_cmac_128bit_key_t *)cmac_key, (uint8_t *)shared_key,
      sizeof(sgx_ec256_dh_shared_t), (sgx_cmac_128bit_tag_t *)&key_derive_key);
  if (SGX_SUCCESS != se_ret) {
    memset_s(&key_derive_key, sizeof(key_derive_key), 0,
             sizeof(key_derive_key));
    INTERNAL_SGX_ERROR_CODE_CONVERTOR(se_ret);
    return se_ret;
  }
  LOG(INFO) << "cmac key: " << stbox::bytes((const char *)cmac_key, 16);
  LOG(INFO) << "first derive key: "
            << stbox::bytes((const char *)key_derive_key, 16);

  // TODO: note this is quite common, we may optimize this into 1 computation
  /* derivation_buffer = counter(0x01) || label || 0x00 ||
   * output_key_len(0x0080) */
  uint32_t derivation_buffer_length = EC_DERIVATION_BUFFER_SIZE(label_length);
  uint8_t *p_derivation_buffer = (uint8_t *)malloc(derivation_buffer_length);
  if (p_derivation_buffer == NULL) {
    return SGX_ERROR_OUT_OF_MEMORY;
  }
  memset(p_derivation_buffer, 0, derivation_buffer_length);

  /*counter = 0x01 */
  p_derivation_buffer[0] = 0x01;
  /*label*/
  memcpy(&p_derivation_buffer[1], label, label_length);
  /*output_key_len=0x0080*/
  uint16_t *key_len =
      (uint16_t *)&p_derivation_buffer[derivation_buffer_length - 2];
  *key_len = 0x0080;

  se_ret = sgx_rijndael128_cmac_msg(
      (sgx_cmac_128bit_key_t *)&key_derive_key, p_derivation_buffer,
      derivation_buffer_length, (sgx_cmac_128bit_tag_t *)derived_key);
  memset_s(&key_derive_key, sizeof(key_derive_key), 0, sizeof(key_derive_key));
  free(p_derivation_buffer);
  if (SGX_SUCCESS != se_ret) {
    INTERNAL_SGX_ERROR_CODE_CONVERTOR(se_ret);
  }
  return se_ret;
}
} // namespace crypto
} // namespace stbox
