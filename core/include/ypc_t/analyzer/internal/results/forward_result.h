#pragma once
#include "common/endian.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/helper/parser_type_traits.h"
#include "ypc_t/analyzer/var/data_hash_var.h"
#include "ypc_t/analyzer/var/enclave_hash_var.h"
#include "ypc_t/analyzer/var/encrypted_param_var.h"
#include "ypc_t/analyzer/var/request_key_var.h"
#include "ypc_t/analyzer/var/result_var.h"

namespace ypc {
namespace internal {
template <typename Crypto>
class forward_result : virtual public request_key_var<true>,
                       virtual public enclave_hash_var,
                       virtual public result_var,
                       virtual public encrypted_param_var,
                       virtual public data_hash_var {
  typedef Crypto ecc;
  typedef request_key_var<true> request_key_var_t;

public:
  uint32_t generate_result() {
    using ntt = nt<stbox::bytes>;
    if (m_target_enclave_hash.size() == 0) {
      LOG(ERROR) << "target enclave hash not set";
      return stbox::stx_status::forward_target_not_set;
    }

    // 1. gen private key
    stbox::bytes shu_skey;
    auto ret = ecc::gen_private_key(shu_skey);
    if (ret) {
      LOG(ERROR) << "gen_private_key failed: " << stbox::status_string(ret);
      return ret;
    }
    stbox::bytes shu_pkey;
    ret = ecc::generate_pkey_from_skey(shu_skey, shu_pkey);
    if (ret) {
      LOG(ERROR) << "generate_pkey_from_skey failed: "
                 << stbox::status_string(ret);
      return ret;
    }
    // 2. gen forward for private key
    stbox::bytes encrypted_skey;
    ret = ecc::encrypt_message_with_prefix(m_target_dian_pkey, shu_skey,
                                           utc::crypto_prefix_forward,
                                           encrypted_skey);

    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(ret);
      return ret;
    }

    stbox::bytes to_sign_msg = m_target_dian_pkey + m_target_enclave_hash;

    stbox::bytes sig;
    ret = ecc::sign_message(shu_skey, to_sign_msg, sig);
    if (ret) {
      LOG(ERROR) << "sign_message failed: " << stbox::status_string(ret);
      return ret;
    }

    // 3. encrypt result with dian pkey
    std::vector<stbox::bytes> batch;
    batch.push_back(result_var::m_result);
    auto pb = make_bytes<stbox::bytes>::for_package<ntt::batch_data_pkg_t,
                                                    ntt::batch_data>(batch);

    ret = ecc::encrypt_message_with_prefix(
        shu_pkey, pb, utc::crypto_prefix_arbitrary, m_encrypted_result_str);

    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(ret);
      return ret;
    }

    // For safety, we may generate allowance for the encrypted result.
    // Yet, we ignore this since shu_skey is one-time generation.

    // 4. generate hash

    stbox::bytes data_hash;
    ret = ecc::sha3_256(stbox::bytes("Fidelius"), data_hash);
    auto data_hash2 = data_hash + result_var::m_result;
    ret = ecc::sha3_256(data_hash2, data_hash);

    ///
    ntt::forward_result_t result;
    ntt::shu_info_t shu;
    shu.set<ntt::pkey, ntt::encrypted_shu_skey, ntt::shu_forward_signature,
            ntt::enclave_hash>(shu_pkey, encrypted_skey, sig,
                               m_target_enclave_hash);
    result.set<ntt::shu_info>(shu);
    result.set<ntt::data_hash>(data_hash);
    result.set<ntt::encrypted_result>(m_encrypted_result_str);

    typename ypc::cast_obj_to_package<ntt::forward_result_t>::type
        result_package = result;
    m_result_bytes = make_bytes<stbox::bytes>::for_package(result_package);

    return ret;
  }

  uint32_t get_analyze_result_size() { return m_result_bytes.size(); }
  uint32_t get_analyze_result(uint8_t *result, uint32_t size) {
    memcpy(result, m_result_bytes.data(), m_result_bytes.size());
    return stbox::stx_status::success;
  }

protected:
  stbox::bytes m_encrypted_result_str;
  stbox::bytes m_cost_signature_str;
  stbox::bytes m_result_signature_str;

  stbox::bytes m_target_enclave_hash;
  stbox::bytes m_target_dian_pkey;

  stbox::bytes m_result_bytes;
};
} // namespace internal
template <typename Crypto>
using forward_result = internal::forward_result<Crypto>;

template <typename Crypto> struct result_type_traits<forward_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::forward_result_parser;
};

} // namespace ypc
