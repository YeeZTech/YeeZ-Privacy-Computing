#pragma once
#include "common/byte.h"
#include "stbox/stx_status.h"

namespace ypc {
namespace crypto {
namespace internal {
template <typename EC, typename AES, typename ECDH> class aes_gcm_impl {
  typedef EC ecc_t;
  typedef AES aes_t;
  typedef ECDH ecdh_t;
  typedef ::ypc::utc::bytes<uint8_t, ::ypc::utc::byte_encode::raw_bytes> bytes;

public:
  static uint32_t get_encrypt_message_size_with_prefix(uint32_t data_size) {
    return aes_t::get_cipher_size(data_size) + ecc_t::get_public_key_size() +
           aes_t::get_mac_code_size();
  }

  static uint32_t encrypt_message_with_prefix(const uint8_t *public_key,
                                              uint32_t pkey_size,
                                              const uint8_t *data,
                                              uint32_t data_size,
                                              uint32_t prefix, uint8_t *cipher,
                                              uint32_t cipher_size) {
    if (aes_t::get_cipher_size(data_size) + ecc_t::get_public_key_size() +
            aes_t::get_mac_code_size() !=
        cipher_size) {
      return stbox::stx_status::aes_invalid_cipher_size;
    }
    bytes gskey(ecc_t::get_private_key_size());

    // we may cache this for performance
    uint32_t ret = ecc_t::gen_private_key(gskey.size(), gskey.data());
    if (ret) {
      return ret;
    }

    uint8_t *gpkey = cipher + aes_t::get_cipher_size(data_size);
    ret = ecc_t::generate_pkey_from_skey(gskey.data(), gskey.size(), gpkey,
                                         ecc_t::get_public_key_size());
    if (ret) {
      return ret;
    }
    bytes shared_key(ecdh_t::get_ecdh_shared_key_size());
    ret = ecdh_t::ecdh_shared_key(gskey.data(), gskey.size(), public_key,
                                  pkey_size, shared_key.data(),
                                  shared_key.size());
    if (ret) {
      return ret;
    }

    uint8_t *p_out_mac = cipher + aes_t::get_cipher_size(data_size) +
                         ecc_t::get_public_key_size();

    ret = aes_t::encrypt_with_prefix(
        shared_key.data(), shared_key.size(), data, data_size, prefix, cipher,
        aes_t::get_cipher_size(data_size), p_out_mac);
    return ret;
  }

  template <typename BytesType>
  static uint32_t encrypt_message_with_prefix(const BytesType &public_key,
                                              const BytesType &data,
                                              uint32_t prefix,
                                              BytesType &cipher) {
    cipher = BytesType(get_encrypt_message_size_with_prefix(data.size()));

    return encrypt_message_with_prefix(public_key.data(), public_key.size(),
                                       data.data(), data.size(), prefix,
                                       cipher.data(), cipher.size());
  }

  static uint32_t get_decrypt_message_size_with_prefix(uint32_t cipher_size) {
    return aes_t::get_data_size(cipher_size) - ecc_t::get_public_key_size() -
           aes_t::get_mac_code_size();
  }

  static uint32_t decrypt_message_with_prefix(const uint8_t *private_key,
                                              uint32_t private_key_size,
                                              const uint8_t *cipher,
                                              uint32_t cipher_size,
                                              uint8_t *data, uint32_t data_size,
                                              uint32_t prefix) {
    if (aes_t::get_cipher_size(data_size) + ecc_t::get_public_key_size() +
            aes_t::get_mac_code_size() !=
        cipher_size) {
      return stbox::stx_status::aes_invalid_data_size;
    }

    bytes shared_key(ecdh_t::get_ecdh_shared_key_size());

    uint32_t ret = ecdh_t::ecdh_shared_key(
        private_key, private_key_size,
        cipher + aes_t::get_cipher_size(data_size),
        ecc_t::get_public_key_size(), shared_key.data(), shared_key.size());
    if (ret) {
      return ret;
    }

    const uint8_t *p_in_mac = cipher + aes_t::get_cipher_size(data_size) +
                              ecc_t::get_public_key_size();

    ret = aes_t::decrypt_with_prefix(shared_key.data(), shared_key.size(),
                                     cipher, aes_t::get_cipher_size(data_size),
                                     prefix, data, data_size, p_in_mac);

    return ret;
  }
  template <typename BytesType>
  static uint32_t decrypt_message_with_prefix(const BytesType &private_key,
                                              const BytesType &cipher,
                                              BytesType &data,
                                              uint32_t prefix) {
    data = BytesType(get_decrypt_message_size_with_prefix(cipher.size()));
    return decrypt_message_with_prefix(private_key.data(), private_key.size(),
                                       cipher.data(), cipher.size(),
                                       data.data(), data.size(), prefix);
  }
};

template <typename Curve, typename AES, typename ECDH> class aes_impl {
  // todo
};
} // namespace internal
} // namespace crypto
} // namespace ypc
