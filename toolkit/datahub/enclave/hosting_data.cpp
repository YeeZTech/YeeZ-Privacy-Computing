#include "common.h"
#include "common/crypto_prefix.h"
#include "corecommon/datahub/package.h"
#include "enclave_t.h"
#include "sgx_trts.h"
#include "sgx_tseal.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/scope_guard.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/log.h"
using stx_status = stbox::stx_status;
// extern char aad_mac_text[64];
using scope_guard = stbox::scope_guard;
using namespace stbox;

typedef ypc::datahub::data_host<stbox::bytes> dhost_t;

class hosting_data {
public:
  hosting_data() {}

  uint32_t init() {
    auto s = stbox::crypto::get_secp256k1_private_key_size();
    m_skey = stbox::bytes(s);
    stbox::crypto::gen_secp256k1_skey(m_skey.size(), m_skey.data());
    s = stbox::crypto::get_secp256k1_public_key_size();
    m_pkey = stbox::bytes(s);
    stbox::crypto::generate_secp256k1_pkey_from_skey(
        m_skey.data(), m_pkey.data(), m_pkey.size());

    m_data_hash = stbox::eth::keccak256_hash(stbox::bytes("Fidelius"));
    return 0;
  }

  uint32_t get_encrypted_sealed_data_size(const uint8_t *sealed_data,
                                          uint32_t sealed_size) {
    uint32_t data_size = unsealed_data_len(sealed_data, sealed_size);
    return stbox::crypto::get_encrypt_message_size_with_prefix(data_size);
  }

  uint32_t encrypt_sealed_data(const uint8_t *sealed_data,
                               uint32_t sealed_data_size,
                               uint8_t *encrypted_data,
                               uint32_t encrypted_data_size) {

    uint32_t data_size = unsealed_data_len(sealed_data, sealed_data_size);
    stbox::bytes d(data_size);
    sgx_status_t status =
        unseal_data(sealed_data, sealed_data_size, d.data(), d.size());

    stbox::bytes k = m_data_hash + d;
    m_data_hash = stbox::eth::keccak256_hash(k);

    if (status != SGX_SUCCESS) {
      LOG(ERROR) << "unseal_data return " << status;
      return status;
    }

    uint32_t istatus = stbox::crypto::encrypt_message_with_prefix(
        m_pkey.data(), m_pkey.size(), d.data(), d.size(),
        ::ypc::utc::crypto_prefix_host_data, encrypted_data,
        encrypted_data_size);

    return istatus;
  }
  uint32_t get_encrypted_data_credential_size() {
    auto t = get_encrypted_data_credential();
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    t.arch(lm);
    return lm.get_length();
  }

  dhost_t::credential_package_t get_encrypted_data_credential() {
    dhost_t::credential_package_t ret;
    ret.set<dhost_t::pkey>(m_pkey);

    {
      uint32_t len = get_sealed_data_size(m_skey.size());
      stbox::bytes s(len);
      seal_file_data(m_skey.data(), m_skey.size(), s.data(), s.size());
      ret.set<dhost_t::sealed_skey>(std::move(s));
    }

    ret.set<dhost_t::data_hash>(m_data_hash);

    {
      auto len = stbox::crypto::get_secp256k1_signature_size();
      stbox::bytes sig(len);
      stbox::bytes data = m_pkey + m_skey + m_data_hash;
      stbox::crypto::sign_message(m_skey.data(), m_skey.size(), data.data(),
                                  data.size(), sig.data(), sig.size());
      ret.set<dhost_t::signature>(sig);
    }

    return ret;
  }

  static dhost_t::usage_license_package_t generate_data_usage_license(
      const dhost_t::credential_package_t &credential_data,
      const stbox::bytes &encrypted_param, const stbox::bytes &enclave_hash,
      const stbox::bytes &pkey4v, const stbox::bytes &tee_pkey) {
    const stbox::bytes &data_hash = credential_data.get<dhost_t::data_hash>();
    const stbox::bytes &sealed_skey =
        credential_data.get<dhost_t::sealed_skey>();
    uint32_t data_size =
        unsealed_data_len(sealed_skey.data(), sealed_skey.size());
    stbox::bytes skey(data_size);
    sgx_status_t status = unseal_data(sealed_skey.data(), sealed_skey.size(),
                                      skey.data(), skey.size());

    stbox::bytes data =
        encrypted_param + enclave_hash + pkey4v + tee_pkey + data_hash;

    auto len = stbox::crypto::get_secp256k1_signature_size();
    stbox::bytes sig(len);
    stbox::crypto::sign_message(skey.data(), skey.size(), data.data(),
                                data.size(), sig.data(), sig.size());

    dhost_t::usage_license_package_t ret;
    ret.set<dhost_t::signature>(std::move(sig));

    auto eskey_len =
        stbox::crypto::get_encrypt_message_size_with_prefix(skey.size());
    stbox::bytes eskey(eskey_len);

    uint32_t istatus = stbox::crypto::encrypt_message_with_prefix(
        tee_pkey.data(), tee_pkey.size(), skey.data(), skey.size(),
        ::ypc::utc::crypto_prefix_host_data_private_key, eskey.data(),
        eskey.size());

    ret.set<dhost_t::encrypted_skey>(eskey);
    return ret;
  }

protected:
  stbox::bytes m_skey;
  stbox::bytes m_pkey;
  stbox::bytes m_data_hash;
};

std::shared_ptr<hosting_data> g_hd;

uint32_t begin_encrypt_sealed_data() {
  g_hd.reset(new hosting_data());
  g_hd->init();
  return 0;
}
uint32_t end_encrypt_sealed_data() { /*g_hd.reset(nullptr); */
  return 0;
}
uint32_t get_encrypted_sealed_data_size(uint8_t *sealed_data,
                                        uint32_t sealed_size) {
  //! we ignore checking g_hd here for performance.
  return g_hd->get_encrypted_sealed_data_size(sealed_data, sealed_size);
}
uint32_t encrypt_sealed_data(uint8_t *sealed_data, uint32_t in_size,
                             uint8_t *encrypted_data, uint32_t out_size) {
  //! we ignore checking g_hd here for performance.
  return g_hd->encrypt_sealed_data(sealed_data, in_size, encrypted_data,
                                   out_size);
}

uint32_t get_encrypted_data_credential_size() {
  if (!g_hd) {
    LOG(ERROR) << "should call begin_encrypt_sealed_data() first";
    return 0;
  }
  return g_hd->get_encrypted_data_credential_size();
}
uint32_t get_encrypted_data_credential(uint8_t *credential,
                                       uint32_t credential_size) {
  if (!g_hd) {
    LOG(ERROR) << "should call begin_encrypt_sealed_data() first";
    return stbox::stx_status::datahub_not_init;
  }
  auto cred = g_hd->get_encrypted_data_credential();

  ypc::make_bytes<stbox::bytes>::for_package(credential, credential_size, cred);

  return stbox::stx_status::success;
}

uint32_t get_data_usage_license_size() {
  auto len = stbox::crypto::get_secp256k1_signature_size();
  stbox::bytes sig(len);
  dhost_t::usage_license_package_t ret;
  ret.set<dhost_t::signature>(std::move(sig));

  auto skey_size = stbox::crypto::get_secp256k1_private_key_size();
  auto eskey_len =
      stbox::crypto::get_encrypt_message_size_with_prefix(skey_size);
  stbox::bytes eskey(eskey_len);

  ret.set<dhost_t::encrypted_skey>(eskey);

  ff::net::marshaler lm(ff::net::marshaler::length_retriver);
  ret.arch(lm);
  return lm.get_length();
}

uint32_t
generate_data_usage_license(uint8_t *credential, uint32_t credential_size,
                            uint8_t *encrypt_param, uint32_t encrypt_param_size,
                            uint8_t *enclave_hash, uint32_t enclave_hash_size,
                            uint8_t *pkey4v, uint32_t pkey4v_size,
                            uint8_t *tee_pkey, uint32_t tee_pkey_size,
                            uint8_t *license, uint32_t license_size) {
    dhost_t::credential_package_t cred;
    {
      ff::net::marshaler lm((const char *)credential, credential_size,
                            ff::net::marshaler::deseralizer);
      cred.arch(lm);
    }

    auto l = hosting_data::generate_data_usage_license(
        cred, stbox::bytes(encrypt_param, encrypt_param_size),
        stbox::bytes(enclave_hash, enclave_hash_size),
        stbox::bytes(pkey4v, pkey4v_size), stbox::bytes(tee_pkey,
    tee_pkey_size));

    ypc::make_bytes<stbox::bytes>::for_package(license, license_size, l);
  return stbox::stx_status::success;
}
