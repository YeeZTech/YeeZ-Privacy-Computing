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

#include "ypc/corecommon/crypto/group.h"
#include "ypc/terminus/crypto_pack.h"
#include "ypc/corecommon/crypto/crypto_pack.h"
#include <assert.h>
#include <gtest/gtest.h>
#include <stdio.h>

TEST(test_group_add, secp256k1_add_ypc){
  auto crypto = ypc::terminus::intel_sgx_and_eth_compatible();
  auto skey1_b = crypto->gen_ecc_private_key();
  auto pkey1_b = crypto->gen_ecc_public_key_from_private_key(skey1_b);
  auto skey2_b = crypto->gen_ecc_private_key();
  auto pkey2_b = crypto->gen_ecc_public_key_from_private_key(skey2_b);
  ypc::crypto::secp256k1_skey_group::key_t skey1, skey2, skey_sum;
  memcpy(&skey1, skey1_b.data(), sizeof(skey1));
  memcpy(&skey2, skey2_b.data(), sizeof(skey2));
  int ret = ypc::crypto::secp256k1_skey_group::add(skey_sum, skey1, skey2);
  ypc::crypto::secp256k1_pkey_group::key_t pkey1, pkey2, pkey_sum;
  memcpy(&pkey1, pkey1_b.data(), sizeof(pkey1));
  memcpy(&pkey2, pkey2_b.data(), sizeof(pkey2));
  ret = ypc::crypto::secp256k1_pkey_group::add(pkey_sum, pkey1, pkey2);
  ypc::bytes skey_sum_b(skey_sum.data, sizeof(skey_sum.data));
  auto pkey_sum_from_skey = crypto->gen_ecc_public_key_from_private_key(skey_sum_b);
  EXPECT_EQ(memcmp(pkey_sum_from_skey.data(), &pkey_sum, sizeof(pkey_sum_from_skey)), 0);
}

TEST(test_group_add, sm2_add_ypc){
  auto crypto = ypc::terminus::sm_compatible();
  auto skey1_b = crypto->gen_ecc_private_key();
  auto pkey1_b = crypto->gen_ecc_public_key_from_private_key(skey1_b);
  auto skey2_b = crypto->gen_ecc_private_key();
  auto pkey2_b = crypto->gen_ecc_public_key_from_private_key(skey2_b);
  ypc::crypto::sm2_skey_group::key_t skey1, skey2, skey_sum;
  memcpy(&skey1, skey1_b.data(), sizeof(skey1));
  memcpy(&skey2, skey2_b.data(), sizeof(skey2));
  int ret = ypc::crypto::sm2_skey_group::add(skey_sum, skey1, skey2);
  ypc::crypto::sm2_pkey_group::key_t pkey1, pkey2, pkey_sum;
  memcpy(&pkey1, pkey1_b.data(), sizeof(pkey1));
  memcpy(&pkey2, pkey2_b.data(), sizeof(pkey2));
  ret = ypc::crypto::sm2_pkey_group::add(pkey_sum, pkey1, pkey2);
  ypc::bytes skey_sum_b(skey_sum.data, sizeof(skey_sum.data));
  auto pkey_sum_from_skey = crypto->gen_ecc_public_key_from_private_key(skey_sum_b);
  EXPECT_EQ(memcmp(pkey_sum_from_skey.data(), &pkey_sum, sizeof(pkey_sum_from_skey)), 0);
}
