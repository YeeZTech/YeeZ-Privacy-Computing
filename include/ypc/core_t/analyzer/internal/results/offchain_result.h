#pragma once
#include "ypc/common/endian.h"
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/core_t/analyzer/var/enclave_hash_var.h"
#include "ypc/core_t/analyzer/var/encrypted_param_var.h"
#include "ypc/core_t/analyzer/var/request_key_var.h"
#include "ypc/core_t/analyzer/var/result_var.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {
template <typename Crypto>
class offchain_result : virtual public request_key_var<true>,
                        virtual public enclave_hash_var,
                        virtual public result_var,
                        virtual public encrypted_param_var,
                        virtual public data_hash_var {
  typedef Crypto crypto_t;
  typedef request_key_var<true> request_key_var_t;

public:
  uint32_t generate_result() {
    stbox::bytes skey;

    crypto_t::gen_private_key(skey);
    stbox::bytes pkey;
    crypto_t::generate_pkey_from_skey(skey, pkey);

    auto rs = result_var::m_result;

    auto status = crypto_t::encrypt_message_with_prefix(
        pkey, rs, utc::crypto_prefix_arbitrary, m_encrypted_result_str);

    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << status;
      return status;
    }

    stbox::bytes hash_m;
    crypto_t::hash_256(m_encrypted_result_str, hash_m);

    stbox::bytes pkey_a;
    status = crypto_t::generate_pkey_from_skey(m_private_key, pkey_a);

    status = crypto_t::encrypt_message_with_prefix(
        pkey_a, skey, utc::crypto_prefix_arbitrary, m_encrypted_c);

    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << status;
      return status;
    }

    stbox::bytes cost_gas_str(sizeof(m_cost_gas));
    memcpy((uint8_t *)&cost_gas_str[0], (uint8_t *)&m_cost_gas,
           sizeof(m_cost_gas));
    ypc::utc::endian_swap(cost_gas_str);

    auto cost_msg =
        m_encrypted_param + m_data_hash + m_enclave_hash + cost_gas_str;
#ifdef DEBUG
    LOG(INFO) << "cost message to sign: " << cost_msg;
#endif
    status =
        crypto_t::sign_message(m_private_key, cost_msg, m_cost_signature_str);
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for sign cost: " << status;
      return status;
    }

    auto msg = m_encrypted_c + hash_m + m_encrypted_param + m_data_hash +
               cost_gas_str + m_enclave_hash;
#ifdef DEBUG
    LOG(INFO) << "result message to sign: " << msg;
#endif
#ifdef DEBUG
    LOG(INFO) << "sign with private key: " << m_private_key;
#endif
    status = crypto_t::sign_message(m_private_key, msg, m_result_signature_str);

    return static_cast<uint32_t>(status);
  }
  nt<stbox::bytes>::offchain_result_package_t get_result_pkg() {
    using ntt = nt<stbox::bytes>;
    typename ntt::offchain_result_package_t pkg;
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::cost_signature>(m_cost_signature_str);
    pkg.set<ntt::result_encrypt_key>(m_encrypted_c);
    return pkg;
  }

  uint32_t get_analyze_result(uint8_t *result, uint32_t size) {

    auto pkg = get_result_pkg();

    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);
    if (size != lm.get_length()) {
      return stbox::stx_status::out_buffer_length_error;
    }
    ff::net::marshaler ld((char *)result, size, ff::net::marshaler::serializer);
    pkg.arch(ld);
    return stbox::stx_status::success;
  }

  uint32_t get_analyze_result_size() {
    auto pkg = get_result_pkg();
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);
    return lm.get_length();
  }

protected:
  stbox::bytes m_encrypted_c;
  stbox::bytes m_encrypted_result_str;
  stbox::bytes m_cost_signature_str;
  stbox::bytes m_result_signature_str;
};
} // namespace internal
template <typename Crypto>
using offchain_result = internal::offchain_result<Crypto>;

template <typename Crypto> struct result_type_traits<offchain_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::offchain_result_parser;
};
} // namespace ypc
