#pragma once
#include "ypc/common/endian.h"
#include "ypc/stbox/gmssl/sm2.h"
#include "ypc/stbox/tsgx/secp256k1/secp256k1.h"

namespace ypc {
namespace crypto {

struct secp256k1_pkey_group {
  typedef secp256k1_pubkey key_t;
  static int add(key_t &r, const key_t &a, const key_t &b) {
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                      SECP256K1_CONTEXT_SIGN);
    key_t s, t;
    memcpy(&s, &a, sizeof(key_t));
    memcpy(&t, &b, sizeof(key_t));
    // public key endian changed for function `generate_pkey_from_skey`
    ypc::utc::change_pubkey_endian((uint8_t *)&s, sizeof(secp256k1_pubkey));
    ypc::utc::change_pubkey_endian((uint8_t *)&t, sizeof(secp256k1_pubkey));

    const key_t *pkeys[2] = {&s, &t};
    int return_val = secp256k1_ec_pubkey_combine(ctx, &r, pkeys, 2);
    ypc::utc::change_pubkey_endian((uint8_t *)&r, sizeof(secp256k1_pubkey));
    secp256k1_context_destroy(ctx);
    return return_val;
  }
};

struct secp256k1_skey_group {
  typedef struct {
    unsigned char data[32];
  } key_t;
  static int add(key_t &r, const key_t &a, const key_t &b) {
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                      SECP256K1_CONTEXT_SIGN);
    memcpy(&r, &a, sizeof(key_t));
    int return_val = secp256k1_ec_privkey_tweak_add(ctx, r.data, b.data);
    secp256k1_context_destroy(ctx);
    return return_val;
  }
};

struct sm2_pkey_group {
  typedef SM2_POINT key_t;
  static int add(key_t &r, const key_t &a, const key_t &b) {
    return sm2_point_add(&r, &a, &b);
  }
};

struct sm2_skey_group {
  typedef struct {
    uint8_t data[32];
  } key_t;
  static int add(key_t &r, const key_t &a, const key_t &b) {
    SM2_BN skey1_bn, skey2_bn, skey_sum_bn;
    sm2_bn_from_bytes(skey1_bn, a.data);
    sm2_bn_from_bytes(skey2_bn, b.data);
    sm2_fn_add(skey_sum_bn, skey1_bn, skey2_bn);
    sm2_bn_to_bytes(skey_sum_bn, r.data);
    return 0;
  }
};

} // namespace crypto
} // namespace ypc
