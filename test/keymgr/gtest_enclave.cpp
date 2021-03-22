#include "keymgr_sgx_module.h"
#include <cstring>
#include <iostream>
#include <memory>

extern "C" {
void ocall_print_string(const char *buf);
}
void ocall_print_string(const char *buf) { std::cout << buf; }

void print_hex(uint8_t *data, uint32_t len) {
  for (int i = 0; i < len; i++) {
    printf("%02x", data[i]);
  }
  printf("\r\n");
}

int main() {
  auto ptr = std::make_shared<keymgr_sgx_module>("../lib/keymgr_t.signed.so");

  uint32_t pkey_size, skey_size;
  uint8_t *public_key, *private_key;
  ptr->generate_secp256k1_key_pair(&public_key, &pkey_size, &private_key,
                                   &skey_size);

  uint32_t sig_size;
  uint8_t *sig;
  const char *data = "Alice pays Bob 0.1 BTC";
  ptr->sign_message(private_key, skey_size, (uint8_t *)data,
                    strlen(data) * sizeof(char), &sig, &sig_size);

  uint32_t cipher_size;
  uint8_t *cipher;
  ptr->encrypt_message(public_key, pkey_size, (uint8_t *)data,
                       strlen(data) * sizeof(char), &cipher, &cipher_size);

  uint32_t decrypted_size;
  uint8_t *decrypted;
  ptr->decrypt_message(private_key, skey_size, cipher, cipher_size, &decrypted,
                       &decrypted_size);

  uint32_t bp_size;
  uint8_t *backup_private_key;
  ptr->backup_private_key(private_key, skey_size, public_key, pkey_size,
                          &backup_private_key, &bp_size);

  uint32_t sealed_size;
  uint8_t *sealed_key;
  ptr->restore_private_key(backup_private_key, bp_size, private_key, skey_size,
                           &sealed_key, &sealed_size);
  return 0;
}
