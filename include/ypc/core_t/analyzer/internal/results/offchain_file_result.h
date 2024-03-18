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
class offchain_file_result : virtual public request_key_var<true>,
                             virtual public enclave_hash_var,
                             virtual public result_var,
                             virtual public encrypted_param_var,
                             virtual public data_hash_var {
  typedef Crypto crypto;
  typedef request_key_var<true> request_key_var_t;
  using ntt = nt<stbox::bytes>;

public:
  uint32_t generate_result() {
    auto pkg =
        ypc::make_package<typename ntt::offchain_result_package_t>::from_bytes(
            result_var::m_result);
    m_tmp_pkey = pkg.get<ntt::pkey>();
    auto skey = pkg.get<ntt::result_encrypt_key>();
    auto rs = pkg.get<ntt::encrypted_result>();

    auto status = crypto::encrypt_message_with_prefix(
        m_tmp_pkey, rs, utc::crypto_prefix_arbitrary, m_encrypted_result_str);
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << status;
      return status;
    }

    stbox::bytes hash_m;
    crypto::hash_256(m_encrypted_result_str, hash_m);

    stbox::bytes pkey_a;
    status = crypto::generate_pkey_from_skey(m_private_key, pkey_a);
    status = crypto::encrypt_message_with_prefix(
        pkey_a, skey, utc::crypto_prefix_arbitrary, m_encrypted_tmp_skey);
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
    status =
        crypto::sign_message(m_private_key, cost_msg, m_cost_signature_str);
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for sign cost: " << status;
      return status;
    }

    auto msg = m_encrypted_tmp_skey + hash_m + m_encrypted_param + m_data_hash +
               cost_gas_str + m_enclave_hash;
    status = crypto::sign_message(m_private_key, msg, m_result_signature_str);
    return static_cast<uint32_t>(status);
  }

  nt<stbox::bytes>::offchain_result_package_t get_result_pkg() {
    typename ntt::offchain_result_package_t pkg;
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::cost_signature>(m_cost_signature_str);
    pkg.set<ntt::result_encrypt_key>(m_encrypted_tmp_skey);
    pkg.set<ntt::pkey>(m_tmp_pkey);
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
  stbox::bytes m_encrypted_tmp_skey;
  stbox::bytes m_tmp_pkey;
  stbox::bytes m_encrypted_result_str;
  stbox::bytes m_cost_signature_str;
  stbox::bytes m_result_signature_str;
};
} // namespace internal
template <typename Crypto>
using offchain_file_result = internal::offchain_file_result<Crypto>;

template <typename Crypto>
struct result_type_traits<offchain_file_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::offchain_result_parser;
};
} // namespace ypc
