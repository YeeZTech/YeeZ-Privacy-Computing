#include "./test_crypto_module.h"
#include "common/crypto_prefix.h"
#include "enclave_u.h"
#include "sgx_dh.h"
#include "sgx_eid.h"
#include "sgx_error.h"
#include "stbox/eth/util.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_cdef.h"
#include "stbox/usgx/sgx_module.h"
#include "ypc/byte.h"
#include "ypc/ref.h"
#include <iostream>

test_crypto_sgx_module::test_crypto_sgx_module(const char *mod_path)
    : stbox::sgx_module(mod_path) {}

ypc::bytes test_crypto_sgx_module::test_generate_pkey(const ypc::bytes &skey) {
  if (skey.size() != 32) {
    throw std::runtime_error("invalid skey");
  }
  char ret[64];
  auto t = ecall<uint32_t>(::test_generate_pkey, (uint8_t *)&skey[0],
                           (uint8_t *)ret);
  return ypc::bytes(ret, 64);
}
ypc::bytes test_crypto_sgx_module::aes_cmac_msg(const ypc::bytes &p_key,
                                                const ypc::bytes &msg) {
  if (p_key.size() != 16) {
    throw std::runtime_error("invalid p_key");
  }

  char ret[16];
  auto t = ecall<uint32_t>(::aes_cmac_msg, (uint8_t *)&p_key[0],
                           (uint8_t *)&msg[0], msg.size(), (uint8_t *)ret);
  if (t != 0) {
    throw std::runtime_error("ecall failed");
  }
  return ypc::bytes(ret, 16);
}
ypc::bytes test_crypto_sgx_module::aes_gcm_encrypt(const ypc::bytes &key,
                                                   const ypc::bytes &data,
                                                   ypc::bytes &cipher,
                                                   const ypc::bytes &iv,
                                                   const ypc::bytes &aad) {
  if (key.size() != 16) {
    throw std::runtime_error("invalid key");
  }
  char mac[16];
  cipher = ypc::bytes(data.size());

  auto t = ecall<uint32_t>(::aes_gcm_encrypt, (uint8_t *)&key[0],
                           (uint8_t *)&data[0], data.size(),
                           (uint8_t *)&cipher[0], (uint8_t *)&iv[0], iv.size(),
                           (uint8_t *)&aad[0], aad.size(), (uint8_t *)mac);
  if (t != 0) {
    throw std::runtime_error("ecall aes_gcm_encrypt failed");
  }
  return ypc::bytes(mac, 16);
}
ypc::bytes test_crypto_sgx_module::ecdh(const ypc::bytes &pkey,
                                        const ypc::bytes &skey) {
  ypc::bytes shared_key(16);
  if (pkey.size() != 64) {
    std::cout << "pkey size: " << pkey.size() << std::endl;
    throw std::runtime_error("invalid pub key");
  }
  if (skey.size() != 32) {
    throw std::runtime_error("invalid private key");
  }
  auto t = ecall<uint32_t>(::test_ecdh, (uint8_t *)&skey[0],
                           (uint8_t *)&pkey[0], (uint8_t *)&shared_key[0]);
  if (t != 0) {
    std::cout << " ret: " << t << std::endl;
    throw std::runtime_error("invalid ecdh call");
  }
  return shared_key;
}
ypc::bytes test_crypto_sgx_module::encrypt(const ypc::bytes &pkey,
                                           const ypc::bytes &data,
                                           uint32_t prefix) {
  auto len =
      ecall<uint32_t>(::get_encrypt_message_size_with_prefix, data.size());

  ypc::bytes cipher(len);
  auto t = ecall<uint32_t>(::encrypt_message_with_prefix, (uint8_t *)&pkey[0],
                           prefix, (uint8_t *)&data[0], data.size(),
                           (uint8_t *)&cipher[0], cipher.size());
  if (t != 0) {
    std::cout << "ret: " << stbox::status_string(t) << std::endl;
    throw std::runtime_error("invalid encrypt call");
  }
  return cipher;
}

ypc::bytes test_crypto_sgx_module::decrypt(const ypc::bytes &skey,
                                           const ypc::bytes &cipher,
                                           uint32_t prefix) {
  auto len =
      ecall<uint32_t>(::get_decrypt_message_size_with_prefix, cipher.size());
  ypc::bytes data(len);
  auto t = ecall<uint32_t>(::decrypt_message_with_prefix, (uint8_t *)&skey[0],
                           prefix, (uint8_t *)&cipher[0], cipher.size(),
                           (uint8_t *)&data[0], data.size());
  if (t != 0) {
    std::cout << "ret: " << stbox::status_string(t) << std::endl;
    throw std::runtime_error("invalid decrypt call");
  }
  return data;
}
ypc::bytes test_crypto_sgx_module::sign_message(const ypc::bytes &skey,
                                                const ypc::bytes &data) {
  ypc::bytes sig(65);
  auto t =
      ecall<uint32_t>(::test_sign_message, (uint8_t *)&skey[0], 32,
                      (uint8_t *)&data[0], data.size(), (uint8_t *)&sig[0]);
  if (t != 0) {
    std::cout << "ret: " << stbox::status_string(t) << std::endl;
    throw std::runtime_error("invalid sign");
  }
  return sig;
}
bool test_crypto_sgx_module::verify_message(const ypc::bytes &data,
                                            const ypc::bytes &sig,
                                            const ypc::bytes &pkey) {
  auto t =
      ecall<uint32_t>(::test_verify_message, (uint8_t *)&data[0], data.size(),
                      (uint8_t *)&sig[0], sig.size(), (uint8_t *)&pkey[0]);
  return t == 0;
}
