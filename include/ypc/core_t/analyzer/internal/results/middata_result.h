#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/core_t/analyzer/interface/keymgr_interface.h"
#include "ypc/core_t/analyzer/var/data_hash_var.h"
#include "ypc/core_t/analyzer/var/enclave_hash_var.h"
#include "ypc/core_t/analyzer/var/encrypted_param_var.h"
#include "ypc/core_t/analyzer/var/middata_var.h"
#include "ypc/core_t/analyzer/var/request_key_var.h"
#include "ypc/core_t/analyzer/var/result_var.h"
#include "ypc/corecommon/crypto/group.h"
#include "ypc/corecommon/crypto/group_traits.h"
#include "ypc/corecommon/kgt.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {
template <typename Crypto>
class middata_result : virtual public request_key_var<true>,
                       virtual public enclave_hash_var,
                       virtual public result_var,
                       virtual public middata_var,
                       virtual public encrypted_param_var,
                       virtual public keymgr_interface<Crypto>,
                       virtual public data_hash_var {
  typedef Crypto crypto_t;
  typedef request_key_var<true> request_key_var_t;
  typedef keymgr_interface<crypto_t> keymgr_interface_t;
  typedef typename ypc::crypto::group_traits<typename crypto_t::ecc_t>::group_t
      skey_group_t;
  typedef
      typename ypc::crypto::ecc_traits<skey_group_t>::peer_group_t pkey_group_t;

public:
  uint32_t generate_result() {
    stbox::bytes algo_skey_b, mid_skey_b, skey_sum_b, dian_key;
    typename skey_group_t::key_t algo_skey, mid_skey, data_skey, skey_sum;

    LOG(INFO) << "request for user key";
    auto ret = keymgr_interface_t::request_private_key_for_public_key(
        m_mid_pkey, mid_skey_b, dian_key);

    LOG(INFO) << "request for algo key";
    ret = keymgr_interface_t::request_private_key_for_public_key(
        m_algo_pkey, algo_skey_b, dian_key);

    // construct skey generate tree
    LOG(INFO) << "request for data key";
    kgt<pkey_group_t> pkey_kgt(m_data_kgt_pkey);
    LOG(INFO) << "construct pkey_kgt";
    std::unordered_map<stbox::bytes, stbox::bytes> peer;
    for (auto &l : pkey_kgt.leaves()) {
      stbox::bytes data_pkey_b((uint8_t *)&l->key_val, sizeof(l->key_val));
      stbox::bytes data_skey_b;
      ret = keymgr_interface_t::request_private_key_for_public_key(
          data_pkey_b, data_skey_b, dian_key);
      if (peer.find(data_pkey_b) != peer.end()) {
        peer.insert(std::make_pair(data_pkey_b, data_skey_b));
      }
    }
    auto skey_node =
        pkey_kgt.construct_skey_kgt_with_pkey_kgt(pkey_kgt.root(), peer);
    kgt<skey_group_t> skey_kgt(skey_node);
    skey_kgt.calculate_kgt_sum();
    data_skey = skey_kgt.sum();

    memcpy(&algo_skey, algo_skey_b.data(), sizeof(algo_skey));
    memcpy(&mid_skey, mid_skey_b.data(), sizeof(mid_skey));
    int add_ret = skey_group_t::add(skey_sum, data_skey, algo_skey);
    add_ret = skey_group_t::add(skey_sum, skey_sum, mid_skey);

    skey_sum_b = stbox::bytes(skey_sum.data, sizeof(skey_sum));
    ret = crypto_t::generate_pkey_from_skey(skey_sum_b, m_pkey_sum);

    ret = crypto_t::encrypt_message_with_prefix(
        m_pkey_sum, result_var::m_result, utc::crypto_prefix_arbitrary,
        m_encrypted_result_str);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(ret);
      return ret;
    }

    stbox::bytes cost_gas_str(sizeof(m_cost_gas));
    memcpy((uint8_t *)&cost_gas_str[0], (uint8_t *)&m_cost_gas,
           sizeof(m_cost_gas));
    ypc::utc::endian_swap(cost_gas_str);

    auto msg = m_encrypted_param + m_data_hash + m_enclave_hash + m_data_hash +
               cost_gas_str;

    ret = crypto_t::sign_message(skey_sum_b, msg, m_result_signature_str);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for sign cost: " << ret;
      return ret;
    }
    return ret;
  }

  uint32_t get_analyze_result_size() {
    using ntt = nt<stbox::bytes>;
    ntt::middata_result_package_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);
    return lm.get_length();
  }
  uint32_t get_analyze_result(uint8_t *result, uint32_t size) {
    using ntt = nt<stbox::bytes>;
    ntt::middata_result_package_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::result_signature>(m_result_signature_str);
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
  stbox::bytes m_pkey_sum;
};
} // namespace internal
template <typename Crypto>
using middata_result = internal::middata_result<Crypto>;

template <typename Crypto> struct result_type_traits<middata_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::middata_result_parser;
};

} // namespace ypc
