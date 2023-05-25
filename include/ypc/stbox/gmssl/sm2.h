/*
 * Copyright (c) 2014 - 2021 The GmSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the GmSSL Project.
 *    (http://gmssl.org/)"
 *
 * 4. The name "GmSSL Project" must not be used to endorse or promote
 *    products derived from this software without prior written
 *    permission. For written permission, please contact
 *    guanzhi1980@gmail.com.
 *
 * 5. Products derived from this software may not be called "GmSSL"
 *    nor may "GmSSL" appear in their names without prior written
 *    permission of the GmSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the GmSSL Project
 *    (http://gmssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE GmSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE GmSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GMSSL_SM2_H
#define GMSSL_SM2_H

#include <gmssl/sm3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// 比较重要的是哪些是公开接口，公开接口应该做完整的参数检查

typedef uint64_t SM2_BN[8];

int sm2_bn_is_zero(const SM2_BN a);
int sm2_bn_is_one(const SM2_BN a);
int sm2_bn_is_odd(const SM2_BN a);
int sm2_bn_cmp(const SM2_BN a, const SM2_BN b);
int sm2_bn_from_hex(SM2_BN r, const char hex[64]);
int sm2_bn_from_asn1_integer(SM2_BN r, const uint8_t *d, size_t dlen);
int sm2_bn_equ_hex(const SM2_BN a, const char *hex);

void sm2_bn_to_bytes(const SM2_BN a, uint8_t out[32]);
void sm2_bn_from_bytes(SM2_BN r, const uint8_t in[32]);
void sm2_bn_to_hex(const SM2_BN a, char hex[64]);
void sm2_bn_to_bits(const SM2_BN a, char bits[256]);
void sm2_bn_set_word(SM2_BN r, uint32_t a);
void sm2_bn_add(SM2_BN r, const SM2_BN a, const SM2_BN b);
void sm2_bn_sub(SM2_BN ret, const SM2_BN a, const SM2_BN b);
void sm2_bn_rand_range(
    SM2_BN r, const SM2_BN range); // 这个函数需要修改一下，从外部引入随机数

#define sm2_bn_init(r) memset((r), 0, sizeof(SM2_BN))
#define sm2_bn_set_zero(r) memset((r), 0, sizeof(SM2_BN))
#define sm2_bn_set_one(r) sm2_bn_set_word((r), 1)
#define sm2_bn_copy(r, a) memcpy((r), (a), sizeof(SM2_BN))
#define sm2_bn_clean(r) memset((r), 0, sizeof(SM2_BN))

// GF(p)
typedef SM2_BN SM2_Fp;

void sm2_fp_add(SM2_Fp r, const SM2_Fp a, const SM2_Fp b);
void sm2_fp_sub(SM2_Fp r, const SM2_Fp a, const SM2_Fp b);
void sm2_fp_mul(SM2_Fp r, const SM2_Fp a, const SM2_Fp b);
void sm2_fp_exp(SM2_Fp r, const SM2_Fp a, const SM2_Fp e);
void sm2_fp_dbl(SM2_Fp r, const SM2_Fp a);
void sm2_fp_tri(SM2_Fp r, const SM2_Fp a);
void sm2_fp_div2(SM2_Fp r, const SM2_Fp a);
void sm2_fp_neg(SM2_Fp r, const SM2_Fp a);
void sm2_fp_sqr(SM2_Fp r, const SM2_Fp a);
void sm2_fp_inv(SM2_Fp r, const SM2_Fp a);
void sm2_fp_rand(
    SM2_Fp
        r); // 外部提供随机性，如果满足条件就输出，如果不满足条件就哈希一下再输出

#define sm2_fp_init(r) sm2_bn_init(r)
#define sm2_fp_set_zero(r) sm2_bn_set_zero(r)
#define sm2_fp_set_one(r) sm2_bn_set_one(r)
#define sm2_fp_copy(r, a) sm2_bn_copy(r, a)
#define sm2_fp_clean(r) sm2_bn_clean(r)

// GF(n)
typedef SM2_BN SM2_Fn;

void sm2_fn_add(SM2_Fn r, const SM2_Fn a, const SM2_Fn b);
void sm2_fn_sub(SM2_Fn r, const SM2_Fn a, const SM2_Fn b);
void sm2_fn_mul(SM2_Fn r, const SM2_Fn a, const SM2_Fn b);
void sm2_fn_exp(SM2_Fn r, const SM2_Fn a, const SM2_Fn e);
void sm2_fn_neg(SM2_Fn r, const SM2_Fn a);
void sm2_fn_sqr(SM2_Fn r, const SM2_Fn a);
void sm2_fn_inv(SM2_Fn r, const SM2_Fn a);
void sm2_fn_rand(SM2_Fn r);

#define sm2_fn_init(r) sm2_bn_init(r)
#define sm2_fn_set_zero(r) sm2_bn_set_zero(r)
#define sm2_fn_set_one(r) sm2_bn_set_one(r)
#define sm2_fn_copy(r, a) sm2_bn_copy(r, a)
#define sm2_fn_clean(r) sm2_bn_clean(r)

typedef struct {
  SM2_BN X;
  SM2_BN Y;
  SM2_BN Z;
} SM2_JACOBIAN_POINT;

void sm2_jacobian_point_init(SM2_JACOBIAN_POINT *R);
void sm2_jacobian_point_set_xy(SM2_JACOBIAN_POINT *R, const SM2_BN x,
                               const SM2_BN y); // 应该返回错误
void sm2_jacobian_point_get_xy(const SM2_JACOBIAN_POINT *P, SM2_BN x, SM2_BN y);
void sm2_jacobian_point_neg(SM2_JACOBIAN_POINT *R, const SM2_JACOBIAN_POINT *P);
void sm2_jacobian_point_dbl(SM2_JACOBIAN_POINT *R, const SM2_JACOBIAN_POINT *P);
void sm2_jacobian_point_add(SM2_JACOBIAN_POINT *R, const SM2_JACOBIAN_POINT *P,
                            const SM2_JACOBIAN_POINT *Q);
void sm2_jacobian_point_sub(SM2_JACOBIAN_POINT *R, const SM2_JACOBIAN_POINT *P,
                            const SM2_JACOBIAN_POINT *Q);
void sm2_jacobian_point_mul(SM2_JACOBIAN_POINT *R, const SM2_BN k,
                            const SM2_JACOBIAN_POINT *P);
void sm2_jacobian_point_to_bytes(const SM2_JACOBIAN_POINT *P, uint8_t out[64]);
void sm2_jacobian_point_from_bytes(SM2_JACOBIAN_POINT *P, const uint8_t in[64]);
void sm2_jacobian_point_mul_generator(SM2_JACOBIAN_POINT *R, const SM2_BN k);
void sm2_jacobian_point_mul_sum(SM2_JACOBIAN_POINT *R, const SM2_BN t,
                                const SM2_JACOBIAN_POINT *P,
                                const SM2_BN s); // 应该返回错误
void sm2_jacobian_point_from_hex(SM2_JACOBIAN_POINT *P,
                                 const char hex[64 * 2]); // 应该返回错误

int sm2_jacobian_point_is_at_infinity(const SM2_JACOBIAN_POINT *P);
int sm2_jacobian_point_is_on_curve(const SM2_JACOBIAN_POINT *P);
int sm2_jacobian_point_equ_hex(const SM2_JACOBIAN_POINT *P,
                               const char hex[128]);

#define sm2_jacobian_point_set_infinity(R) sm2_jacobian_point_init(R)
#define sm2_jacobian_point_copy(R, P)                                          \
  memcpy((R), (P), sizeof(SM2_JACOBIAN_POINT))

/*
SM2 Public API

SM2接口有两个层次，基本的和ASN.1/PKI的
基本的接口不依赖ASN.1编码，可以直接将结构体的内存输出(endian一致即可)
基本的接口也不进行输入的格式检查，调用方应保证输入不为空
*/

// 这里应该用#define 给出常量的值
extern const SM2_BN SM2_P;
// extern const SM2_BN SM2_A;
extern const SM2_BN SM2_B;
extern const SM2_BN SM2_N;
extern const SM2_BN SM2_ONE;
extern const SM2_BN SM2_TWO;
extern const SM2_BN SM2_THREE;
extern const SM2_BN SM2_U_PLUS_ONE;
extern const SM2_JACOBIAN_POINT *SM2_G; // 应该同时给出Affine的

typedef struct {
  uint8_t x[32];
  uint8_t y[32];
} SM2_POINT;

void sm2_point_to_compressed_octets(const SM2_POINT *P, uint8_t out[33]);
void sm2_point_to_uncompressed_octets(const SM2_POINT *P, uint8_t out[65]);
int sm2_point_from_octets(SM2_POINT *P, const uint8_t *in, size_t inlen);

int sm2_point_from_x(SM2_POINT *P, const uint8_t x[32], int y);
int sm2_point_from_xy(SM2_POINT *P, const uint8_t x[32], const uint8_t y[32]);
int sm2_point_is_on_curve(const SM2_POINT *P);
int sm2_point_add(SM2_POINT *R, const SM2_POINT *P, const SM2_POINT *Q);
int sm2_point_mul(SM2_POINT *R, const uint8_t k[32], const SM2_POINT *P);
int sm2_point_mul_generator(SM2_POINT *R, const uint8_t k[32]);
int sm2_point_mul_sum(SM2_POINT *R, const uint8_t k[32], const SM2_POINT *P,
                      const uint8_t s[32]); // R = k * P + s * G

/*
RFC 5480 Elliptic Curve Cryptography Subject Public Key Information
ECPoint ::= OCTET STRING
*/
#define SM2_POINT_MAX_SIZE (2 + 65)
int sm2_point_to_der(const SM2_POINT *P, uint8_t **out, size_t *outlen);
int sm2_point_from_der(SM2_POINT *P, const uint8_t **in, size_t *inlen);

typedef struct {
  SM2_POINT public_key;
  uint8_t private_key[32];
} SM2_KEY;

int sm2_key_generate(SM2_KEY *key);
int sm2_key_set_private_key(SM2_KEY *key,
                            const uint8_t private_key[32]); // 自动生成公钥
int sm2_key_set_public_key(
    SM2_KEY *key,
    const SM2_POINT *public_key); // 自动清空私钥，不要和set_private_key同时用

int sm2_public_key_equ(const SM2_KEY *sm2_key, const SM2_KEY *pub_key);
// int sm2_public_key_copy(SM2_KEY *sm2_key, const SM2_KEY *pub_key); //
// 这个函数的逻辑不清楚
int sm2_public_key_digest(const SM2_KEY *key, uint8_t dgst[32]);

/*
from RFC 5915

ECPrivateKey ::= SEQUENCE {
        version		INTEGER,	-- value MUST be (1)
        privateKey	OCTET STRING,	-- big endian encoding of integer
这里不是以INTEGER编码的，因此长度固定
        parameters	[0] EXPLICIT ECParameters OPTIONAL,
                                        -- ONLY namedCurve OID is permitted, by
RFC 5480
                                        -- MUST always include this field, by
RFC 5915 publicKey	[1] EXPLICIT BIT STRING OPTIONAL -- compressed_point
                                        -- SHOULD always include this field, by
RFC 5915 }

ECParameters ::= CHOICE { namedCurve OBJECT IDENTIFIER }
*/
#define SM2_PRIVATE_KEY_DEFAULT_SIZE 120 // generated
#define SM2_PRIVATE_KEY_BUF_SIZE 512     // MUST >= SM2_PRIVATE_KEY_DEFAULT_SIZE

int sm2_private_key_to_der(const SM2_KEY *key, uint8_t **out, size_t *outlen);
int sm2_private_key_from_der(SM2_KEY *key, const uint8_t **in, size_t *inlen);

/*
AlgorithmIdentifier ::= {
        algorithm	OBJECT IDENTIFIER { id-ecPublicKey },
        parameters	OBJECT IDENTIFIER { id-sm2 } }
*/
int sm2_public_key_algor_to_der(uint8_t **out, size_t *outlen);
int sm2_public_key_algor_from_der(const uint8_t **in, size_t *inlen);

/*
X.509 SubjectPublicKeyInfo from RFC 5280

SubjectPublicKeyInfo  ::=  SEQUENCE  {
        algorithm            AlgorithmIdentifier,
        subjectPublicKey     BIT STRING  -- uncompressed octets of ECPoint }
*/
int sm2_public_key_info_to_der(const SM2_KEY *a, uint8_t **out, size_t *outlen);
int sm2_public_key_info_from_der(SM2_KEY *a, const uint8_t **in, size_t *inlen);

/*
PKCS #8 PrivateKeyInfo from RFC 5208

PrivateKeyInfo ::= SEQUENCE {
        version			Version { v1(0) },
        privateKeyAlgorithm	AlgorithmIdentifier,
        privateKey		OCTET STRING, -- DER-encoding of ECPrivateKey
        attributes		[0] IMPLICIT SET OF Attribute OPTIONAL }
*/
enum {
  PKCS8_private_key_info_version = 0,
};

int sm2_private_key_info_to_der(const SM2_KEY *key, uint8_t **out,
                                size_t *outlen);
int sm2_private_key_info_from_der(SM2_KEY *key, const uint8_t **attrs,
                                  size_t *attrslen, const uint8_t **in,
                                  size_t *inlen);

/*
EncryptedPrivateKeyInfo ::= SEQUENCE {
        encryptionAlgorithm	EncryptionAlgorithmIdentifier, -- id-PBES2
        encryptedData		OCTET STRING }
*/
int sm2_private_key_info_encrypt_to_der(const SM2_KEY *key, const char *pass,
                                        uint8_t **out, size_t *outlen);
int sm2_private_key_info_decrypt_from_der(SM2_KEY *key, const uint8_t **attrs,
                                          size_t *attrs_len, const char *pass,
                                          const uint8_t **in, size_t *inlen);

typedef struct {
  uint8_t r[32];
  uint8_t s[32];
} SM2_SIGNATURE;

int sm2_do_sign(const SM2_KEY *key, const uint8_t dgst[32], SM2_SIGNATURE *sig);
int sm2_do_verify(const SM2_KEY *key, const uint8_t dgst[32],
                  const SM2_SIGNATURE *sig);

#define SM2_MIN_SIGNATURE_SIZE 8
#define SM2_MAX_SIGNATURE_SIZE 72
int sm2_signature_to_der(const SM2_SIGNATURE *sig, uint8_t **out,
                         size_t *outlen);
int sm2_signature_from_der(SM2_SIGNATURE *sig, const uint8_t **in,
                           size_t *inlen);
int sm2_sign(const SM2_KEY *key, const uint8_t dgst[32], uint8_t *sig,
             size_t *siglen);
int sm2_verify(const SM2_KEY *key, const uint8_t dgst[32], const uint8_t *sig,
               size_t siglen);

#define SM2_DEFAULT_ID "1234567812345678"
#define SM2_DEFAULT_ID_LENGTH                                                  \
  (sizeof(SM2_DEFAULT_ID) - 1) // LENGTH for string and SIZE for bytes
#define SM2_DEFAULT_ID_BITS (SM2_DEFAULT_ID_LENGTH * 8)
#define SM2_MAX_ID_BITS 65535
#define SM2_MAX_ID_LENGTH (SM2_MAX_ID_BITS / 8)

int sm2_compute_z(uint8_t z[32], const SM2_POINT *pub, const char *id,
                  size_t idlen);

typedef struct {
  SM3_CTX sm3_ctx;
  SM2_KEY key;
} SM2_SIGN_CTX;

int sm2_sign_init(SM2_SIGN_CTX *ctx, const SM2_KEY *key, const char *id,
                  size_t idlen);
int sm2_sign_update(SM2_SIGN_CTX *ctx, const uint8_t *data, size_t datalen);
int sm2_sign_finish(SM2_SIGN_CTX *ctx, uint8_t *sig, size_t *siglen);

int sm2_verify_init(SM2_SIGN_CTX *ctx, const SM2_KEY *key, const char *id,
                    size_t idlen);
int sm2_verify_update(SM2_SIGN_CTX *ctx, const uint8_t *data, size_t datalen);
int sm2_verify_finish(SM2_SIGN_CTX *ctx, const uint8_t *sig, size_t siglen);

/*
SM2Cipher ::= SEQUENCE {
        XCoordinate	INTEGER,
        YCoordinate	INTEGER,
        HASH		OCTET STRING SIZE(32),
        CipherText	OCTET STRING }
*/
#define SM2_MIN_PLAINTEXT_SIZE                                                 \
  1 // re-compute SM2_MIN_CIPHERTEXT_SIZE when modify
#define SM2_MAX_PLAINTEXT_SIZE                                                 \
  255 // re-compute SM2_MAX_CIPHERTEXT_SIZE when modify

typedef struct {
  SM2_POINT point;
  uint8_t hash[32];
  uint8_t ciphertext_size;
  uint8_t ciphertext[SM2_MAX_PLAINTEXT_SIZE];
} SM2_CIPHERTEXT;

int sm2_do_encrypt(const SM2_KEY *key, const uint8_t *in, size_t inlen,
                   SM2_CIPHERTEXT *out);
int sm2_do_decrypt(const SM2_KEY *key, const SM2_CIPHERTEXT *in, uint8_t *out,
                   size_t *outlen);

#define SM2_MIN_CIPHERTEXT_SIZE 45  // dependes on SM2_MIN_PLAINTEXT_SIZE
#define SM2_MAX_CIPHERTEXT_SIZE 366 // depends on SM2_MAX_PLAINTEXT_SIZE
int sm2_ciphertext_to_der(const SM2_CIPHERTEXT *c, uint8_t **out,
                          size_t *outlen);
int sm2_ciphertext_from_der(SM2_CIPHERTEXT *c, const uint8_t **in,
                            size_t *inlen);
int sm2_encrypt(const SM2_KEY *key, const uint8_t *in, size_t inlen,
                uint8_t *out, size_t *outlen);
int sm2_decrypt(const SM2_KEY *key, const uint8_t *in, size_t inlen,
                uint8_t *out, size_t *outlen);

int sm2_ecdh(const SM2_KEY *key, const SM2_POINT *peer_public, SM2_POINT *out);

#ifdef __cplusplus
}
#endif
#endif
