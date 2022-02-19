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
class onchain_result : virtual public request_key_var<true>,
                       virtual public enclave_hash_var,
                       virtual public result_var,
                       virtual public encrypted_param_var,
                       virtual public data_hash_var {
  typedef Crypto ecc;
  typedef request_key_var<true> request_key_var_t;

public:
  uint32_t generate_result() {
    auto status = ecc::encrypt_message_with_prefix(
        request_key_var_t::m_pkey4v, result_var::m_result,
        utc::crypto_prefix_arbitrary, m_encrypted_result_str);

    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: "
                 << stbox::status_string(status);
      return status;
    }
    stbox::bytes cost_gas_str(sizeof(m_cost_gas));
    memcpy((uint8_t *)&cost_gas_str[0], (uint8_t *)&m_cost_gas,
           sizeof(m_cost_gas));
    ypc::utc::endian_swap(cost_gas_str);

    auto cost_msg =
        m_encrypted_param + m_data_hash + m_enclave_hash + cost_gas_str;
    status = ecc::sign_message(m_private_key, cost_msg, m_cost_signature_str);

    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for sign cost: " << status;
      return status;
    }

    auto msg = m_encrypted_param + m_data_hash + m_enclave_hash + cost_gas_str +
               m_encrypted_result_str;
    status = ecc::sign_message(m_private_key, msg, m_result_signature_str);
    return static_cast<uint32_t>(status);
  }

  uint32_t get_analyze_result_size() {
    using ntt = nt<stbox::bytes>;
    ntt::onchain_result_package_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::cost_signature>(m_cost_signature_str);
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);
    return lm.get_length();
  }
  uint32_t get_analyze_result(uint8_t *result, uint32_t size) {
    using ntt = nt<stbox::bytes>;
    ntt::onchain_result_package_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::cost_signature>(m_cost_signature_str);
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);
    if (size != lm.get_length()) {
      return stbox::stx_status::out_buffer_length_error;
    }
    ff::net::marshaler ld((char *)result, size, ff::net::marshaler::serializer);
    pkg.arch(ld);
    return stbox::stx_status::success;
  }

protected:
  stbox::bytes m_encrypted_result_str;
  stbox::bytes m_cost_signature_str;
  stbox::bytes m_result_signature_str;
};
} // namespace internal
template <typename Crypto>
using onchain_result = internal::onchain_result<Crypto>;

template <typename Crypto>
struct result_type_traits<onchain_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::onchain_result_parser;
};
} // namespace ypc
