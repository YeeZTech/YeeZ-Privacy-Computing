#pragma once
#include "stbox/ebyte.h"
#include "stbox/tsgx/crypto/raw_ecc.h"

namespace stbox {
namespace crypto {

template <typename Curve> struct ecc {
  typedef raw_ecc<Curve> raw_t;
  static uint32_t get_private_key_size() {
    return raw_t::get_private_key_size();
  }
  static uint32_t get_public_key_size() { return raw_t::get_public_key_size(); }
  static uint32_t gen_private_key(bytes &skey) {
    skey = bytes(get_private_key_size());
    return raw_t::gen_private_key(skey.size(), skey.data());
  }

  static uint32_t generate_pkey_from_skey(const bytes &skey, bytes &pkey) {
    pkey = bytes(get_public_key_size());
    return raw_t::generate_pkey_from_skey(skey.data(), skey.size(), pkey.data(),
                                          pkey.size());
  }
  static uint32_t get_signature_size() { return raw_t::get_signature_size(); }

  static uint32_t sign_message(const bytes &skey, const bytes &data,
                               bytes &sig) {
    sig = bytes(get_signature_size());
    return raw_t::sign_message(skey.data(), skey.size(), data.data(),
                               data.size(), sig.data(), sig.size());
  }

  static uint32_t verify_signature(const bytes &data, const bytes &sig,
                                   const bytes &public_key) {
    return raw_t::verify_signature(data.data(), data.size(), sig.data(),
                                   sig.size(), public_key.data(),
                                   public_key.size());
  }

  static uint32_t get_encrypt_message_size_with_prefix(uint32_t data_size) {
    return raw_t::get_encrypt_message_size_with_prefix(data_size);
  }
  static uint32_t encrypt_message_with_prefix(const bytes &public_key,
                                              const bytes &data,
                                              uint32_t prefix, bytes &cipher) {
    cipher = bytes(get_encrypt_message_size_with_prefix(data.size()));

    return raw_t::encrypt_message_with_prefix(
        public_key.data(), public_key.size(), data.data(), data.size(), prefix,
        cipher.data(), cipher.size());
  }

  static uint32_t get_decrypt_message_size_with_prefix(uint32_t data_size) {
    return raw_t::get_decrypt_message_size_with_prefix(data_size);
  }
  static uint32_t decrypt_message_with_prefix(const bytes &private_key,
                                              const bytes &cipher, bytes &data,
                                              uint32_t prefix) {
    data = bytes(get_decrypt_message_size_with_prefix(cipher.size()));
    return raw_t::decrypt_message_with_prefix(
        private_key.data(), private_key.size(), cipher.data(), cipher.size(),
        data.data(), data.size(), prefix);
  }
};
} // namespace crypto
} // namespace stbox
