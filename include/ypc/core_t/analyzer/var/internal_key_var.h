#pragma once
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {
template <typename Crypto>
class internal_key_var : virtual public analyzer_context {
  typedef Crypto ecc;
public:
  typedef nt<stbox::bytes> ntt;

  internal_key_var(){
    auto ret = ecc::gen_private_key(m_private_key);
    if(ret){
      LOG(ERROR)<<"gen_private_key failed: "<<stbox::status_string(ret);
      throw std::runtime_error("gen_private_key failed");
    }

    ret = ecc::generate_pkey_from_skey(m_private_key, m_public_key);
    if (ret) {
      LOG(ERROR) << "generate_pkey_from_skey failed: "
                 << stbox::status_string(ret);
      throw std::runtime_error("generate_pkey_from_skey failed");
    }
  };
  virtual const stbox::bytes &get_internal_public_key() const {
    return m_public_key;
  }
  virtual const stbox::bytes &get_internal_private_key() const {
    return m_private_key;
  }

  virtual ntt::shu_info_t
  forward_internal_key(const stbox::bytes &dian_pkey,
                       const stbox::bytes &target_enclave_hash) {
    stbox::bytes encrypted_skey;
    auto ret = ecc::encrypt_message_with_prefix(
        dian_pkey, m_private_key, utc::crypto_prefix_forward, encrypted_skey);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(ret);
      throw std::runtime_error("error for encrypt_message");
    }

    stbox::bytes to_sign_msg = dian_pkey + target_enclave_hash;
    stbox::bytes sig;
    ret = ecc::sign_message(m_private_key, to_sign_msg, sig);
    if (ret) {
      LOG(ERROR) << "sign_message failed: " << stbox::status_string(ret);
      throw std::runtime_error("error for sign_message");
    }
    ntt::shu_info_t shu;
    shu.set<ntt::pkey, ntt::encrypted_shu_skey, ntt::shu_forward_signature,
            ntt::enclave_hash>(m_public_key, encrypted_skey, sig,
                               target_enclave_hash);
    return shu;
  }

  virtual stbox::bytes export_internal_key(const stbox::bytes &shu_pkey) {
    stbox::bytes m_encrypted_skey;
    auto ret = ecc::encrypt_message_with_prefix(shu_pkey, m_private_key,
                                                utc::crypto_prefix_arbitrary,
                                                m_encrypted_skey);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(ret);
      throw std::runtime_error("encrypt_message failed");
    }
    return m_encrypted_skey;
  }

protected:
  stbox::bytes m_private_key;
  stbox::bytes m_public_key;
};
} // namespace internal
} // namespace ypc
