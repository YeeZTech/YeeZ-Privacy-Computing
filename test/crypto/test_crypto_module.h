#pragma once
#include "common/crypto_prefix.h"
#include "stbox/usgx/sgx_module.h"
#include "ypc/byte.h"
#include <iostream>

using stx_status = stbox::stx_status;
class test_crypto_sgx_module : public stbox::sgx_module {
public:
  test_crypto_sgx_module(const char *mod_path);

  ypc::bytes test_generate_pkey(const ypc::bytes &skey);
  ypc::bytes aes_cmac_msg(const ypc::bytes &p_key, const ypc::bytes &msg);
  ypc::bytes aes_gcm_encrypt(const ypc::bytes &key, const ypc::bytes &data,
                             ypc::bytes &cipher, const ypc::bytes &iv,
                             const ypc::bytes &aad);
  ypc::bytes ecdh(const ypc::bytes &pkey, const ypc::bytes &skey);
  ypc::bytes encrypt(const ypc::bytes &pkey, const ypc::bytes &data,
                     uint32_t prefix);

  ypc::bytes decrypt(const ypc::bytes &skey, const ypc::bytes &cipher,
                     uint32_t prefix);
  ypc::bytes sign_message(const ypc::bytes &skey, const ypc::bytes &data);
  bool verify_message(const ypc::bytes &data, const ypc::bytes &sig,
                      const ypc::bytes &pkey);
};
