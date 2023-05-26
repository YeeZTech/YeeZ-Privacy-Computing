//#pragma once
#if defined(_WIN32)
/*
 * The defined WIN32_NO_STATUS macro disables return code definitions in
 * windows.h, which avoids "macro redefinition" MSVC warnings in ntstatus.h.
 */
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <bcrypt.h>
#include <ntstatus.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/random.h>
#elif defined(__OpenBSD__)
#include <unistd.h>
#else
#error "Couldn't identify the OS"
#endif

#include <limits.h>
#include <stddef.h>

#include "ypc/core/group.h"
#include <assert.h>
#include <gtest/gtest.h>
#include <stdio.h>

/* Returns 1 on success, and 0 on failure. */
int fill_random(unsigned char *data, size_t size) {
#if defined(_WIN32)
  NTSTATUS res =
      BCryptGenRandom(NULL, data, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  if (res != STATUS_SUCCESS || size > ULONG_MAX) {
    return 0;
  } else {
    return 1;
  }
#elif defined(__linux__) || defined(__FreeBSD__)
  /* If `getrandom(2)` is not available you should fallback to /dev/urandom */
  ssize_t res = getrandom(data, size, 0);
  if (res < 0 || (size_t)res != size) {
    return 0;
  } else {
    return 1;
  }
#elif defined(__APPLE__) || defined(__OpenBSD__)
  /* If `getentropy(2)` is not available you should fallback to either
   * `SecRandomCopyBytes` or /dev/urandom */
  int res = getentropy(data, size);
  if (res == 0) {
    return 1;
  } else {
    return 0;
  }
#endif
  return 0;
}
TEST(test_group_add, secp256k1_add) {
#ifdef YPC_SGX
  printf("YPC_SGX defined\n");
#else
  printf("YPC_SGX undefined\n");
#endif
  for (int i = 0; i < 100; i++) {
    unsigned char randomize[32];
    secp256k1_skey_group::key_t skey1, skey2, skey_sum;
    int return_val;
    secp256k1_pubkey pkey1, pkey2, pkey_sum_from_skey, pkey_sum_from_pkey;

    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                      SECP256K1_CONTEXT_SIGN);
    return_val = fill_random(randomize, sizeof(randomize));
    EXPECT_EQ(return_val, 1);
    while (1) {
      return_val = fill_random(skey1.data, sizeof(skey1));
      EXPECT_EQ(return_val, 1);
      return_val = fill_random(skey2.data, sizeof(skey2));
      EXPECT_EQ(return_val, 1);
      if (secp256k1_ec_seckey_verify(ctx, skey1.data) &&
          secp256k1_ec_seckey_verify(ctx, skey2.data)) {
        break;
      }
    }

    return_val = secp256k1_ec_pubkey_create(ctx, &pkey1, skey1.data);
    assert(return_val);
    return_val = secp256k1_ec_pubkey_create(ctx, &pkey2, skey2.data);
    assert(return_val);
    return_val = secp256k1_skey_group::add(skey_sum, skey1, skey2);

    return_val =
        secp256k1_ec_pubkey_create(ctx, &pkey_sum_from_skey, skey_sum.data);
    assert(return_val);

    return_val = secp256k1_pkey_group::add(pkey_sum_from_pkey, pkey1, pkey2);
    secp256k1_context_destroy(ctx);
    EXPECT_EQ(memcmp(pkey_sum_from_skey.data, pkey_sum_from_pkey.data, 64), 0);
  }
}

TEST(test_group_add, sm2_add) {
  for (int i = 0; i < 100; i++) {
    SM2_KEY sm2_key1;
    int return_val = sm2_key_generate(&sm2_key1);
    EXPECT_EQ(return_val, 1);

    SM2_KEY sm2_key2;
    return_val = sm2_key_generate(&sm2_key2);
    EXPECT_EQ(return_val, 1);

    sm2_skey_group::key_t skey_sum, skey1, skey2;
    memcpy(skey2.data, sm2_key2.private_key, 32);
    memcpy(skey1.data, sm2_key1.private_key, 32);
    return_val = sm2_skey_group::add(skey_sum, skey1, skey2);

    SM2_KEY sm2key_sum;
    return_val = sm2_key_set_private_key(&sm2key_sum, skey_sum.data);

    SM2_POINT pkey_sum_point;
    return_val = sm2_pkey_group::add(pkey_sum_point, sm2_key1.public_key,
                                     sm2_key2.public_key);
    EXPECT_EQ(memcmp(&(sm2key_sum.public_key), &pkey_sum_point,
                     sizeof(pkey_sum_point)),
              0);
  }
}
