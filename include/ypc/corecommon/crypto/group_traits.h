#pragma once
#include "gmssl/sm2_ecc.h"
#include "group.h"
#include "stdeth/secp256k1.h"

namespace ypc {
namespace crypto {

// group traits
template <typename T> struct group_traits {
};
template <> struct group_traits<secp256k1> {
  typedef secp256k1_skey_group group_t;
};
template <> struct group_traits<sm2_ecc> { typedef sm2_skey_group group_t; };

// ecc traits
template <typename T> struct ecc_traits {};
template <> struct ecc_traits<secp256k1_pkey_group> {
  typedef secp256k1 ecc_t;
  typedef secp256k1_skey_group peer_group_t;
  static constexpr bool is_private_key_type = false;
};
template <> struct ecc_traits<secp256k1_skey_group> {
  typedef secp256k1 ecc_t;
  typedef secp256k1_pkey_group peer_group_t;
  static constexpr bool is_private_key_type = true;
};
template <> struct ecc_traits<sm2_pkey_group> {
  typedef sm2_ecc ecc_t;
  typedef sm2_skey_group peer_group_t;
  static constexpr bool is_private_key_type = false;
};
template <> struct ecc_traits<sm2_skey_group> {
  typedef sm2_ecc ecc_t;
  typedef sm2_pkey_group peer_group_t;
  static constexpr bool is_private_key_type = true;
};
} // namespace crypto
} // namespace ypc
