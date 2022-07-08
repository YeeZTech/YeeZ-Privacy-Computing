/*
 * Copyright (c) 2014 - 2020 The GmSSL Project.  All rights reserved.
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

#include <string.h>
#include <gmssl/sm2.h>
#include <gmssl/asn1.h>


int sm2_key_generate(SM2_KEY *key)
{
	SM2_BN x;
	SM2_BN y;
	SM2_JACOBIAN_POINT _P, *P = &_P;

	if (!key) {
		return -1;
	}
	memset(key, 0, sizeof(SM2_KEY));

	do {
		sm2_bn_rand_range(x, SM2_N);
	} while (sm2_bn_is_zero(x));
	sm2_bn_to_bytes(x, key->private_key);

	sm2_jacobian_point_mul_generator(P, x);
	sm2_jacobian_point_get_xy(P, x, y);
	sm2_bn_to_bytes(x, key->public_key.x);
	sm2_bn_to_bytes(y, key->public_key.y);
	return 1;
}

int sm2_key_set_private_key(SM2_KEY *key, const uint8_t private_key[32])
{
	memcpy(&key->private_key, private_key, 32);
	if (sm2_point_mul_generator(&key->public_key, private_key) != 1) {
		return -1;
	}
	return 1;
}

int sm2_key_set_public_key(SM2_KEY *key, const SM2_POINT *public_key)
{
	if (!key || !public_key) {
		return -1;
	}
	memset(key, 0, sizeof(SM2_KEY));
	key->public_key = *public_key;
	return 1;
}

