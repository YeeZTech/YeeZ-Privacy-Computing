#pragma once
#include "ypc/stbox/tsgx/secp256k1/secp256k1.h"
#include <gmssl/sm2.h>
#include <memory>
#include <ypc/core/byte.h>

struct secp256k1_pkey_group {
  typedef secp256k1_pubkey key_t;
  static int add(key_t &r, const key_t &a, const key_t &b) {
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                      SECP256K1_CONTEXT_SIGN);
    const key_t *pkeys[2] = {&a, &b};
    key_t t;
    int return_val = secp256k1_ec_pubkey_combine(ctx, &t, pkeys, 2);
    memcpy(&r, &t, sizeof(key_t));
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
