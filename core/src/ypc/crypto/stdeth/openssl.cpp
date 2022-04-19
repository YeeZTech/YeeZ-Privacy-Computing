#include "openssl.h"
#include <openssl/cmac.h>

namespace ypc {
namespace openssl {

int sgx::rijndael128_cmac_msg(const uint8_t *cmac_128bit_key,
                              const uint8_t *p_src, uint32_t src_len,
                              uint8_t *mac_128bit) {
  void *pState = NULL;

  if ((cmac_128bit_key == NULL) || (p_src == NULL) || (mac_128bit == NULL)) {
    return 1;
  }

  size_t mactlen;
  int ret = 2;

  do {
    // create a new ctx of CMAC
    //
    pState = CMAC_CTX_new();
    if (pState == NULL) {
      ret = 3;
      break;
    }

    // init CMAC ctx with the corresponding size, key and AES alg.

    if (!CMAC_Init((CMAC_CTX *)pState, (const void *)cmac_128bit_key, 16,
                   EVP_aes_128_cbc(), NULL)) {
      break;
    }

    // perform CMAC hash on p_src
    //
    if (!CMAC_Update((CMAC_CTX *)pState, p_src, src_len)) {
      break;
    }

    // finalize CMAC hashing
    //
    if (!CMAC_Final((CMAC_CTX *)pState, (unsigned char *)mac_128bit,
                    &mactlen)) {
      break;
    }

    // validate mac size
    //
    if (mactlen != 16) {
      break;
    }

    ret = 0;
  } while (0);

  // we're done, clear and free CMAC ctx
  if (pState) {
    CMAC_CTX_free((CMAC_CTX *)pState);
  }
  return ret;
}

int sgx::rijndael128GCM_encrypt(const uint8_t *aes_gcm_128bit_key,
                                const uint8_t *p_src, uint32_t src_len,
                                uint8_t *p_dst, const uint8_t *p_iv,
                                uint32_t iv_len, const uint8_t *p_aad,
                                uint32_t aad_len,
                                uint8_t *aes_gcm_128_bit_out_mac) {
  EVP_add_cipher(EVP_aes_128_gcm());
  EVP_CIPHER_CTX_free_ptr m_ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);

  int len = 0;
  int rc = EVP_EncryptInit_ex(m_ctx.get(), EVP_aes_128_gcm(), NULL, NULL, NULL);
  if (rc != 1) {
    throw std::runtime_error("EVP_EncryptInit_ex failed");
  }

  if (1 !=
      EVP_CIPHER_CTX_ctrl(m_ctx.get(), EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL)) {
    throw std::runtime_error("EVP_CIPHER_CTX_ctrl failed");
  }

  if (1 !=
      EVP_EncryptInit_ex(m_ctx.get(), NULL, NULL, aes_gcm_128bit_key, p_iv)) {
    throw std::runtime_error("EVP_EncryptInit_ex failed");
  }

  if (1 != EVP_EncryptUpdate(m_ctx.get(), NULL, &len, p_aad, aad_len)) {
    throw std::runtime_error("EVP_EncryptUpdate for add failed");
  }
  if (1 != EVP_EncryptUpdate(m_ctx.get(), p_dst, &len, p_src, src_len)) {
    throw std::runtime_error("EVP_EncryptUpdate failed");
  }

  /*Finalise the encryption. Nothing happens in GCM mode*/
  if (1 != EVP_EncryptFinal_ex(m_ctx.get(), p_dst + len, &len)) {
    throw std::runtime_error("EVP_EncryptFinal_ex failed");
  }

  // get the tag
  if (1 != EVP_CIPHER_CTX_ctrl(m_ctx.get(), EVP_CTRL_GCM_GET_TAG, 16,
                               aes_gcm_128_bit_out_mac)) {
    throw std::runtime_error("EVP_CIPHER_CTX_ctrl failed");
  }
  return 0;
  }

  int sgx::rijndael128GCM_decrypt(const uint8_t *aes_gcm_128bit_key,
                                  const uint8_t *p_src, uint32_t src_len,
                                  uint8_t *p_dst, const uint8_t *p_iv,
                                  uint32_t iv_len, const uint8_t *p_aad,
                                  uint32_t aad_len,
                                  const uint8_t *aes_gcm_128bit_in_mac) {
    EVP_add_cipher(EVP_aes_128_gcm());
    EVP_CIPHER_CTX_free_ptr m_ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);

    int rc =
        EVP_EncryptInit_ex(m_ctx.get(), EVP_aes_128_gcm(), NULL, NULL, NULL);
    int len = 0;
    if (rc != 1) {
      throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (!EVP_CIPHER_CTX_ctrl(m_ctx.get(), EVP_CTRL_GCM_SET_IVLEN, iv_len,
                             NULL)) {
      throw std::runtime_error("EVP_CIPHER_CTX_ctrl failed");
    }

    if (!EVP_DecryptInit_ex(m_ctx.get(), NULL, NULL, aes_gcm_128bit_key,
                            p_iv)) {
      throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    if (!EVP_DecryptUpdate(m_ctx.get(), NULL, &len, p_aad, aad_len)) {
      throw std::runtime_error("EVP_DecryptUpdate failed");
    }

    if (!EVP_DecryptUpdate(m_ctx.get(), p_dst, &len, p_src, src_len)) {
      throw std::runtime_error("EVP_DecryptUpdate failed");
    }

    auto plaintext_len = len;
    if (!EVP_CIPHER_CTX_ctrl(m_ctx.get(), EVP_CTRL_GCM_SET_TAG, 16,
                             (void *)aes_gcm_128bit_in_mac)) {
      throw std::runtime_error("EVP_CIPHER_CTX_ctrl failed");
    }
    auto ret = EVP_DecryptFinal_ex(m_ctx.get(), p_dst + len, &len);
    if (ret > 0) {
      return 0;
    } else {
      LOG(ERROR) << "invalid data";
      return 1;
    }
  }

  int sgx::sha256_msg(const uint8_t *message, size_t size,
                      uint8_t *sha256_hash) {
    std::unique_ptr<EVP_MD_CTX, decltype(&::EVP_MD_CTX_free)> ctx(
        EVP_MD_CTX_new(), ::EVP_MD_CTX_free);

    if (1 != EVP_DigestInit_ex(ctx.get(), EVP_sha256(), NULL)) {
      throw std::runtime_error("EVP_DigestInit_ex failed");
    }

    if (1 != EVP_DigestUpdate(ctx.get(), message, size)) {
      throw std::runtime_error("EVP_DigestUpdate failed");
    }

    unsigned int digest_len;
    if (1 != EVP_DigestFinal_ex(ctx.get(), sha256_hash, &digest_len)) {
      throw std::runtime_error("EVP_DigestFinal_ex failed");
    }
    if (digest_len != 32) {
      throw std::runtime_error("sha256_msg unexpected length");
    }
    return 0;
  }
} // namespace openssl
} // namespace ypc
