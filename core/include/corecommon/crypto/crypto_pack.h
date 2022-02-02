#pragma once
#include "corecommon/crypto/aes_gcm_traits.h"
#include "corecommon/crypto/interface/aes_impl.h"
#include "corecommon/crypto/interface/ecc_impl.h"
#include "corecommon/crypto/interface/ecdh_impl.h"
#include "corecommon/crypto/interface/hash_impl.h"
#include "corecommon/crypto/interface/sign_impl.h"

namespace ypc {
namespace crypto {

template <typename EC, typename Hash, typename AES, typename ECDH,
          bool use_gcm = aes_gcm_traits<AES>::value>
struct crypto_pack {};

template <typename EC, typename Hash, typename AES, typename ECDH>
struct crypto_pack<EC, Hash, AES, ECDH, true>
    : public internal::ecc_impl<EC>,
      public internal::hash_impl<Hash>,
      public internal::sign_impl<Hash, EC>,
      public internal::ecdh_impl<ECDH>,
      public internal::aes_gcm_impl<EC, AES, ECDH> {
  typedef EC ecc_t;
  typedef Hash hash_t;
  typedef AES aes_t;
  typedef ECDH ecdh_t;
};

template <typename EC, typename Hash, typename AES, typename ECDH>
struct crypto_pack<EC, Hash, AES, ECDH, false>
    : public internal::ecc_impl<EC>,
      public internal::hash_impl<Hash>,
      public internal::sign_impl<Hash, EC>,
      public internal::ecdh_impl<ECDH>,
      public internal::aes_impl<EC, AES, ECDH> {
  typedef EC ecc_t;
  typedef Hash hash_t;
  typedef AES aes_t;
  typedef ECDH ecdh_t;
};

} // namespace crypto
} // namespace ypc
