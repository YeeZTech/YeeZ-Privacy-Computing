#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/core_t/analyzer/var/data_hash_var.h"
#include "ypc/core_t/analyzer/var/enclave_hash_var.h"
#include "ypc/core_t/analyzer/var/encrypted_param_var.h"
#include "ypc/core_t/analyzer/var/request_key_var.h"
#include "ypc/core_t/analyzer/var/result_var.h"
#include "ypc/core_t/analyzer/var/taskgraph_pkey_var.h"
#include "ypc/corecommon/crypto/group.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {
template <typename Crypto>
class taskgraph_result : virtual public enclave_hash_var,
                         virtual public result_var,
                         virtual public encrypted_param_var,
                         virtual public taskgraph_pkey_var,
                         virtual public data_hash_var {
  typedef Crypto ecc;

public:
  uint32_t generate_result() {
    stbox::bytes data_skey_b, algo_skey_b, mid_skey_b, dian_key, skey_sum_b;
    ecc::skey_group_t::key_t skey_sum, data_skey, algo_skey, mid_skey;
    uint32_t ret = keymgr_interface_t::request_private_key_for_public_key(
        m_data_pkey, data_skey_b, dian_key);
    ret = keymgr_interface_t::request_private_key_for_public_key(
        m_algo_pkey, algo_skey_b, dian_key);
    ret = keymgr_interface_t::request_private_key_for_public_key(
        m_mid_pkey, mid_skey_b, dian_key);

    memcpy(data_skey, data_skey_b.data(), sizeof(data_skey));
    memcpy(algo_skey, algo_skey_b.data(), sizeof(algo_skey));
    memcpy(mid_skey, mid_skey_b.data(), sizeof(mid_skey));
    int add_ret = ecc::skey_group_t::add(skey_sum, data_skey, algo_skey);
    add_ret = ecc::skey_group_t::add(skey_sum, skey_sum, mid_skey);

    memcpy(skey_sum_b.data(), skey_sum, skey_sum_b.size());
    ret = ecc::generate_pkey_from_skey(skey_sum_b, m_pkey_sum);

    ret = ecc::encrypt_message_with_prefix(pkey_sum_b, result_var::m_result,
                                           utc::crypto_prefix_arbitrary,
                                           m_encrypted_result_str);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(ret);
      return ret;
    }

    stbox::bytes cost_gas_str(sizeof(m_cost_gas));
    memcpy((uint8_t *)&cost_gas_str[0], (uint8_t *)&m_cost_gas,
           sizeof(m_cost_gas));
    ypc::utc::endian_swap(cost_gas_str);

    auto msg = m_encrypted_param + m_data_hash + m_enclave_hash + hash_m +
               cost_gas_str;

    ret = ecc::sign_message(skey_sum_b, msg, m_result_signature_str);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for sign cost: " << ret;
      return ret;
    }
    return ret;
  }

  uint32_t get_analyze_result_size() {
    using ntt = nt<stbox::bytes>;
    ntt::taskgraph_result_package_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::cost_gas>(result_var::m_cost_gas);
    pkg.set<ntt::pkey_sum>(m_pkey_sum);
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);
    return lm.get_length();
  }
  uint32_t get_analyze_result(uint8_t *result, uint32_t size) {
    using ntt = nt<stbox::bytes>;
    ntt::taskgraph_result_pkg_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::pkey_sum>(m_pkey_sum);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::cost_gas>(result_var::m_cost_gas);
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
  stbox::bytes m_result_signature_str;
  stbox::bytes m_hash;
  stbox::bytes m_pkey_sum;

} // namespace internal
template <typename Crypto>
using taskgraph_result = internal::taskgraph_result<Crypto>;

template <typename Crypto> struct result_type_traits<taskgraph_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::taskgraph_result_parser;
};

} // namespace ypc
