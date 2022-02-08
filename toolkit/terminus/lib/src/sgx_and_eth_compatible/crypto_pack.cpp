#include "ypc/terminus/crypto_pack.h"
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_ecdh.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"
extern "C" {
#include "vendor/keccak/keccak.h"
}
#include "common/endian.h"
#include "openssl.h"
#include "zallocator.h"
#include <openssl/rand.h>

#define SECP256K1_PRIVATE_KEY_SIZE 32
#define AAD_MAC_TEXT_LEN 64
#define AAD_MAC_PREFIX_POS 24
#define INITIALIZATION_VECTOR_SIZE 12
#define EC_DERIVATION_BUFFER_SIZE(label_length) ((label_length) + 4)
static char aad_mac_text[AAD_MAC_TEXT_LEN] = "tech.yeez.key.manager";
static uint8_t p_iv_text[INITIALIZATION_VECTOR_SIZE] = {
    89, 101, 101, 90, 70, 105, 100, 101, 108, 105, 117, 115}; //"YeeZFidelius";

static int ecdh_hash_function_sha256(unsigned char *output,
                                     const unsigned char *x,
                                     const unsigned char *y, void *data) {
  uint8_t buf[33];
  unsigned char version = (y[31] & 0x01) | 0x02;
  buf[0] = version;
  memcpy(buf + 1, x, 32);
  ypc::openssl::sgx sgx;
  sgx.sha256_msg(buf, 33, output);
  return 1;
}

#define MAC_KEY_SIZE 16
static uint8_t cmac_key[MAC_KEY_SIZE] = "yeez.tech.stbox";
#define EC_DERIVATION_BUFFER_SIZE(label_length) ((label_length) + 4)

int derive_key(const uint8_t *shared_key, size_t shared_key_len,
               const char *label, uint32_t label_length,
               uint8_t *derived_128bit_key) {
  int se_ret = 0;
  ypc::bytes key_derive_key(16);

  // memset(cmac_key, 0, MAC_KEY_SIZE);
  ::ypc::openssl::sgx sgx;

  se_ret = sgx.rijndael128_cmac_msg(cmac_key, shared_key, shared_key_len,
                                    key_derive_key.data());
  if (se_ret) {
    return se_ret;
  }

  uint32_t derivation_buffer_length = EC_DERIVATION_BUFFER_SIZE(label_length);
  ypc::bytes p_derivation_buffer(derivation_buffer_length);
  memset(p_derivation_buffer.data(), p_derivation_buffer.size(), 0);
  /*counter = 0x01 */
  p_derivation_buffer[0] = 0x01;
  /*label*/
  memcpy(&p_derivation_buffer[1], label, label_length);
  /*output_key_len=0x0080*/
  p_derivation_buffer[p_derivation_buffer.size() - 3] = 0;
  uint16_t *key_len =
      (uint16_t *)&p_derivation_buffer[derivation_buffer_length - 2];
  *key_len = 0x0080;

  se_ret = sgx.rijndael128_cmac_msg(
      key_derive_key.data(), p_derivation_buffer.data(),
      derivation_buffer_length, derived_128bit_key);
  if (se_ret) {
    return se_ret;
  }
  return 0;
}

namespace ypc {
namespace terminus {
class intel_sgx_and_eth_compatible_pack : public crypto_pack {
public:
  intel_sgx_and_eth_compatible_pack() {
    m_ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                     SECP256K1_CONTEXT_SIGN);
  }
  virtual ~intel_sgx_and_eth_compatible_pack() {
    secp256k1_context_destroy(m_ctx);
  }

  virtual bytes gen_ecc_private_key() {
    uint32_t skey_size = SECP256K1_PRIVATE_KEY_SIZE;
    uint8_t skey[SECP256K1_PRIVATE_KEY_SIZE];

    int counter = 0;
    do {
      int rc = RAND_bytes(skey, skey_size);
      if (rc != 1) {
        throw std::runtime_error("RAND_bytes key failed");
      }
      counter++;
      if (counter > 0xFFFF) {
        throw std::runtime_error("too many times, you may retry");
      }

    } while (!secp256k1_ec_seckey_verify(m_ctx, skey));
    return bytes(skey, skey_size);
  }

  virtual bytes gen_ecc_public_key_from_private_key(const bytes &private_key) {
    secp256k1_pubkey pkey;
    auto ret = secp256k1_ec_pubkey_create(m_ctx, &pkey, private_key.data());
    if (!ret) {
      LOG(ERROR) << "Pubkey computation failed: " << ret;
      return bytes();
    }
    ::ypc::utc::change_pubkey_endian((uint8_t *)&pkey,
                                     sizeof(secp256k1_pubkey));
    return bytes((uint8_t *)&pkey, sizeof(secp256k1_pubkey));
  }

  virtual bytes sign_hash(const bytes &hash, const bytes &private_key) {
    secp256k1_ecdsa_recoverable_signature rsig;
    auto ret = secp256k1_ecdsa_sign_recoverable(m_ctx, &rsig, hash.data(),
                                                private_key.data(), NULL, NULL);
    if (!ret) {
      LOG(ERROR) << "sign error: " << ret;
      return bytes();
    }

    int recid;
    bytes sig(1 + sizeof(secp256k1_ecdsa_signature));

    ret = secp256k1_ecdsa_recoverable_signature_serialize_compact(
        m_ctx, sig.data(), &recid, &rsig);
    if (!ret) {
      LOG(ERROR) << "serialize sig error: " << ret;
      return bytes();
    }
    sig[64] = (uint8_t)(recid + 27);
    return sig;
  }
  virtual bool verify_hash_signature(const bytes &sig, const bytes &hash,
                                     const bytes &pubkey) {
    if (pubkey.size() != sizeof(secp256k1_pubkey)) {
      return false;
    }

    secp256k1_pubkey secp256k1_pkey;
    memcpy(&secp256k1_pkey, pubkey.data(), pubkey.size());
    ::ypc::utc::change_pubkey_endian((uint8_t *)&secp256k1_pkey,
                                     sizeof(secp256k1_pubkey));

    secp256k1_ecdsa_recoverable_signature rsig;
    auto se_ret = secp256k1_ecdsa_recoverable_signature_parse_compact(
        m_ctx, &rsig, sig.data(), *(sig.data() + 64) - 27);
    if (!se_ret) {
      LOG(ERROR)
          << "secp256k1_ecdsa_recoverable_signature_parse_compact return "
          << (uint32_t)se_ret;
      return false;
    }
    secp256k1_ecdsa_signature secp256k1_sig;
    se_ret = secp256k1_ecdsa_recoverable_signature_convert(
        m_ctx, &secp256k1_sig, &rsig);
    if (!se_ret) {
      LOG(ERROR) << "secp256k1_ecdsa_recoverable_signature_convert return "
                 << (uint32_t)se_ret;
      return false;
    }

    // auto hash = stbox::eth::msg_hash(data, data_size);
    se_ret = secp256k1_ecdsa_verify(m_ctx, &secp256k1_sig, hash.data(),
                                    &secp256k1_pkey);
    if (!se_ret) {
      LOG(ERROR) << "secp256k1_ecdsa_verify return " << (uint32_t)se_ret;
      return false;
    }
    return true;
  }

  virtual bytes chain_msg(const bytes &message) {
    bytes msg({0x19});
    bytes tmp("Ethereum Signed Message:\n32");
    msg = msg + tmp;
    auto raw_hash = hash(message);
    msg = msg + raw_hash;
    return msg;
  }

  virtual bytes chain_hash(const bytes &message) {
    auto msg = chain_msg(message);
    return hash(msg);
  }

  virtual bytes hash(const bytes &msg) {
    sha3_context c;
    uint8_t *hash;
    sha3_Init256(&c);
    sha3_SetFlags(&c, SHA3_FLAGS_KECCAK);
    sha3_Update(&c, msg.data(), msg.size());
    hash = (uint8_t *)sha3_Finalize(&c);
    return bytes(hash, 32);
  }

  virtual bytes ecdh_key(const bytes &private_key, const bytes &pubkey) {
    secp256k1_pubkey lpkey;
    memcpy((uint8_t *)&lpkey, pubkey.data(), sizeof(secp256k1_pubkey));
    ::ypc::utc::change_pubkey_endian((uint8_t *)&lpkey,
                                     sizeof(secp256k1_pubkey));

    bytes ec256_dh_shared_key(32);
    auto se_ret =
        secp256k1_ecdh(m_ctx, ec256_dh_shared_key.data(), &lpkey,
                       private_key.data(), ecdh_hash_function_sha256, NULL);


    if (!se_ret) {
      LOG(ERROR) << "secp256k1_ecdh returns: " << (uint32_t)se_ret;
      return bytes();
    }
    uint32_t aad_mac_len = strlen(aad_mac_text);
    bytes derived_key(16);

    int ret = derive_key(ec256_dh_shared_key.data(), ec256_dh_shared_key.size(),
                         aad_mac_text, aad_mac_len, derived_key.data());
    if (ret) {
      LOG(ERROR) << "derive_key failed";
      return bytes();
    }
    return derived_key;
  }

  virtual bytes ecc_encrypt(const bytes &msg, const bytes &public_key,
                            uint32_t prefix) {

    if (msg.size() == 0) {
      return bytes();
    }
    auto skey = gen_ecc_private_key();
    if (skey.size() == 0) {
      return bytes();
    }
    auto pkey = gen_ecc_public_key_from_private_key(skey);
    if (pkey.size() == 0) {
      return bytes();
    }

    bytes cipher(msg.size() + pkey.size() + 16);

    memcpy(cipher.data() + msg.size(), pkey.data(), pkey.size());

    auto derived_key = ecdh_key(skey, public_key);
    if (derived_key.size() == 0) {
      return bytes();
    }

    uint8_t mac_text[AAD_MAC_TEXT_LEN];
    memset(mac_text, 0, AAD_MAC_TEXT_LEN);
    memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
    uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
    *p_prefix = prefix;

    ypc::openssl::sgx sgx;
    uint8_t *p_out_mac = cipher.data() + msg.size() + pkey.size();
    auto se_ret = sgx.rijndael128GCM_encrypt(
        derived_key.data(), msg.data(), msg.size(), cipher.data(), p_iv_text,
        INITIALIZATION_VECTOR_SIZE, mac_text, AAD_MAC_TEXT_LEN, p_out_mac);
    if (se_ret) {
      return bytes();
    }
    return cipher;
  }

  virtual bytes ecc_decrypt(const bytes &cipher, const bytes &private_key,
                            uint32_t prefix) {

    auto data_size = cipher.size() - 16 - sizeof(secp256k1_pubkey);
    auto pkey = bytes(cipher.data() + data_size, sizeof(secp256k1_pubkey));
    auto derived_key = ecdh_key(private_key, pkey);
    if (derived_key.size() == 0) {
      LOG(ERROR) << "derived_key invalid";
      return bytes();
    }

    uint8_t mac_text[AAD_MAC_TEXT_LEN];
    memset(mac_text, 0, AAD_MAC_TEXT_LEN);
    memcpy(mac_text, aad_mac_text, AAD_MAC_TEXT_LEN);
    uint32_t *p_prefix = (uint32_t *)(mac_text + AAD_MAC_PREFIX_POS);
    *p_prefix = prefix;

    bytes data(data_size);
    ypc::openssl::sgx sgx;

    auto se_ret = sgx.rijndael128GCM_decrypt(
        derived_key.data(), cipher.data(), data_size, data.data(), p_iv_text,
        INITIALIZATION_VECTOR_SIZE, mac_text, AAD_MAC_TEXT_LEN,
        (cipher.data() + data_size + sizeof(secp256k1_pubkey)));
    if (se_ret) {
      LOG(ERROR) << "sgx.rijndael128GCM_decrypt failed";
      return bytes();
    }
    return data;
  }

protected:
  secp256k1_context *m_ctx;
};

std::unique_ptr<crypto_pack> intel_sgx_and_eth_compatible() {
  return std::unique_ptr<crypto_pack>(new intel_sgx_and_eth_compatible_pack());
}
} // namespace terminus
} // namespace ypc
