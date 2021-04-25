#include "common/crypto_prefix.h"
#include "common/util.h"
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

using stx_status = stbox::stx_status;
class test_crypto_sgx_module : public stbox::sgx_module {
public:
  test_crypto_sgx_module(const char *mod_path) : stbox::sgx_module(mod_path) {}

  std::string test_generate_pkey(const std::string &skey) {
    if (skey.size() != 32) {
      throw std::runtime_error("invalid skey");
    }
    char ret[64];
    auto t = ecall<uint32_t>(::test_generate_pkey, (uint8_t *)&skey[0],
                             (uint8_t *)ret);
    return std::string(ret, 64);
  }
  std::string aes_cmac_msg(const std::string &p_key, const std::string &msg) {
    if (p_key.size() != 16) {
      throw std::runtime_error("invalid p_key");
    }

    char ret[16];
    auto t = ecall<uint32_t>(::aes_cmac_msg, (uint8_t *)&p_key[0],
                             (uint8_t *)&msg[0], msg.size(), (uint8_t *)ret);
    if (t != 0) {
      throw std::runtime_error("ecall failed");
    }
    return std::string(ret, 16);
  }
  std::string aes_gcm_encrypt(const std::string &key, const std::string &data,
                              std::string &cipher, const std::string &iv,
                              const std::string &aad) {
    if (key.size() != 16) {
      throw std::runtime_error("invalid key");
    }
    char mac[16];
    cipher = std::string(data.size(), '\0');

    auto t = ecall<uint32_t>(
        ::aes_gcm_encrypt, (uint8_t *)&key[0], (uint8_t *)&data[0], data.size(),
        (uint8_t *)&cipher[0], (uint8_t *)&iv[0], iv.size(), (uint8_t *)&aad[0],
        aad.size(), (uint8_t *)mac);
    if (t != 0) {
      throw std::runtime_error("ecall aes_gcm_encrypt failed");
    }
    return std::string(mac, 16);
  }
  std::string ecdh(const std::string &pkey, const std::string &skey) {
    std::string shared_key(16, '\0');
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
  std::string encrypt(const std::string &pkey, const std::string &data,
                      uint32_t prefix) {
    auto len =
        ecall<uint32_t>(::get_encrypt_message_size_with_prefix, data.size());

    std::string cipher(len, '\0');
    auto t = ecall<uint32_t>(::encrypt_message_with_prefix, (uint8_t *)&pkey[0],
                             prefix, (uint8_t *)&data[0], data.size(),
                             (uint8_t *)&cipher[0], cipher.size());
    if (t != 0) {
      std::cout << "ret: " << t << std::endl;
      throw std::runtime_error("invalid encrypt call");
    }
    return cipher;
  }

  std::string decrypt(const std::string &skey, const std::string &cipher,
                      uint32_t prefix) {
    auto len =
        ecall<uint32_t>(::get_decrypt_message_size_with_prefix, cipher.size());
    std::string data(len, '\0');
    auto t = ecall<uint32_t>(::decrypt_message_with_prefix, (uint8_t *)&skey[0],
                             prefix, (uint8_t *)&cipher[0], cipher.size(),
                             (uint8_t *)&data[0], data.size());
    if (t != 0) {
      std::cout << "ret: " << t << std::endl;
      throw std::runtime_error("invalid decrypt call");
    }
    return data;
  }
};

int main(int argc, char *argv[]) {
  test_crypto_sgx_module tcsm("../lib/test_crypto_enclave.signed.so");

  // test case from https://github.com/allan-stewart/node-aes-cmac
  std::string p_key("k3Men*p/2.3j4abB");
  std::string msg("this|is|a|test|message");
  std::string result = tcsm.aes_cmac_msg(p_key, msg);

  std::string expected = "0125c538f8be7c4eea370f992a4ffdcb";
  if (ypc::encode_hex(result) != expected) {
    std::cout << "cmac msg: " << ypc::encode_hex(result) << std::endl;
    std::cout << "not match" << std::endl;
  } else {
    std::cout << "test cmac msg success!" << std::endl;
  }

  p_key = "3zTvzr3p67VC61jm";
  std::string iv = "60iP0h6vJoEa";
  std::string aad = "tech.yeez.key.manager";
  msg = "hello world";
  std::string cipher;

  std::string mac = tcsm.aes_gcm_encrypt(p_key, msg, cipher, iv, aad);
  std::cout << "ecoded hex: " << ypc::encode_hex(cipher) << std::endl;
  std::cout << "mac: " << ypc::encode_hex(mac) << std::endl;

  /***************************generate pub key*****************/
  std::string skey =
      "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043";
  expected = "5d7ee992f48ffcdb077c2cb57605b602bd4029faed3e91189c7fb9fccc72771e4"
             "5b7aa166766e2ad032d0a195372f5e2d20db792901d559ab0d2bfae10ecea97";
  // std::cout << "before pkey size: " << pkey.size() << std::endl;

  auto t1 = ypc::bytes::from_hex(skey);
  skey = ypc::byte_to_string(t1);

  std::string pkey = tcsm.test_generate_pkey(skey);
  if (expected == ypc::encode_hex(pkey)) {
    std::cout << "generate pkey succ" << std::endl;
  } else {
    std::cout << "!!!!generate pkey failed" << std::endl;
  }
  /***************************ecdh*****************/
  pkey =
      "a042a0ea6d9c0f1ec0f8563c0292a35635f538ba3bd1bc690e282eeaa0a3c744c3e7e2"
      "e25228881222ce014b7ac087d797b8e4482e9b4e243f5db0ad1e0fc266";
  auto t2 = ypc::bytes::from_hex(pkey);
  pkey = ypc::byte_to_string(t2);

  std::string shared_key = tcsm.ecdh(pkey, skey);
  expected = "226c29a1a26844169ad4e6ce33bcb7bd";
  if (expected == ypc::encode_hex(shared_key)) {
    std::cout << "ecdh succ" << std::endl;
  } else {
    std::cout << "!!!!ecdh failed" << std::endl;
    std::cout << "shared key: " << ypc::encode_hex(shared_key);
    return 0;
  }
  /***************************encrypt/decrypt*****************/
  std::string data = "hello world";

  pkey = tcsm.test_generate_pkey(skey);
  std::cout << "start encrypt" << std::endl;
  cipher = tcsm.encrypt(pkey, data, ypc::crypto_prefix_forward);
  std::cout << "cipher: " << ypc::encode_hex(cipher) << std::endl;

  std::cout << "start decrypt" << std::endl;
  std::string dd = tcsm.decrypt(skey, cipher, ypc::crypto_prefix_forward);
  if (dd != data) {
    std::cout << "!!!!decrypt and encrypt are inconsistency" << std::endl;
    return 0;
  } else {
    std::cout << "encrypt decrypt succ" << std::endl;
  }

  /***************************decrypt sepcific data*****************/
  std::string cipher_str =
      "ae3cb833a1a32126fa2dd5f8622616352350dc005eb7bc478524ac0b9ff77161734d97f3"
      "6e89a2e7a931e672b982d2c56b9ab8c1248dea269576c301b088e266ff63302fce18c07a"
      "24c0640efd8844bafb5f1101c35b65444425be";
  std::string new_cipher =
      ypc::byte_to_string(ypc::bytes::from_hex(cipher_str));
  std::cout << "new cipher: " << ypc::encode_hex(new_cipher) << std::endl;
  dd = tcsm.decrypt(skey, new_cipher, ypc::crypto_prefix_arbitrary);
  if (dd != data) {
    std::cout << "!!!!decrypt and encrypt are inconsistency" << std::endl;
    return 0;
  } else {
    std::cout << "encrypt decrypt succ" << std::endl;
  }

  return 0;
}
