#include "ypc/terminus/crypto_pack.h"
#include "corecommon/crypto/crypto_pack.h"
#include "corecommon/crypto/stdeth.h"

namespace ypc {
namespace terminus {
template <typename Crypto> class crypto_pack_t : public crypto_pack {
public:
  typedef Crypto crypto_t;

  virtual bytes gen_ecc_private_key() {
    bytes ret;
    auto status = crypto_t::gen_private_key(ret);
    if (status) {
      std::cerr << "gen_ecc_private_key return " << std::hex << status
                << std::endl;
      throw std::runtime_error("terminus lib gen_ecc_private_key fail");
    }
    return ret;
  }
  virtual bytes gen_ecc_public_key_from_private_key(const bytes &private_key) {
    bytes ret;
    auto status = crypto_t::generate_pkey_from_skey(private_key, ret);
    if (status) {
      std::cerr << "gen_ecc_public_key_from_private_key return " << std::hex
                << status << std::endl;
      throw std::runtime_error(
          "terminus lib gen_ecc_public_key_from_private_key fail");
    }
    return ret;
  }
  virtual bytes sha3_256(const bytes &msg) {
    bytes sig;
    auto status = crypto_t::sha3_256(msg, sig);
    if (status) {
      std::cerr << "sha3_256 return " << std::hex << status << std::endl;
      throw std::runtime_error("terminus lib sha3_256 fail");
    }
    return sig;
  }

  virtual bytes sign_message(const bytes &msg, const bytes &private_key) {
    bytes ret;
    auto status = crypto_t::sign_message(private_key, msg, ret);
    if (status) {
      std::cerr << "sign_message return " << std::hex << status << std::endl;
      throw std::runtime_error("terminus lib sig_message fail");
    }
    return ret;
  }

  virtual bool verify_message_signature(const bytes &sig, const bytes &message,
                                        const bytes &pubkey) {
    return stbox::stx_status::success ==
           crypto_t::verify_signature(message, sig, pubkey);
  }

  virtual bytes ecc_encrypt(const bytes &msg, const bytes &public_key,
                            uint32_t prefix) {
    bytes ret;
    uint32_t status =
        crypto_t::encrypt_message_with_prefix(public_key, msg, prefix, ret);
    if (status) {
      std::cerr << "ecc_encrypt return " << std::hex << status << std::endl;
      throw std::runtime_error("terminus lib ecc_encrypt fail");
    }
    return ret;
  }

  virtual bytes ecc_decrypt(const bytes &cipher, const bytes &private_key,
                            uint32_t prefix) {
    bytes ret;
    uint32_t status =
        crypto_t::decrypt_message_with_prefix(private_key, cipher, ret, prefix);
    if (status) {
      std::cerr << "ecc_decrypt return " << std::hex << status << std::endl;
      throw std::runtime_error("terminus lib ecc_decrypt fail");
    }
    return ret;
  }
};

std::unique_ptr<crypto_pack> intel_sgx_and_eth_compatible() {
  return std::unique_ptr<crypto_pack>(
      new crypto_pack_t<::ypc::crypto::eth_sgx_crypto>());
}
} // namespace terminus
} // namespace ypc
