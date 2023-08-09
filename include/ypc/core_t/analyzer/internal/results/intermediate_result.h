#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/core_t/analyzer/interface/keymgr_interface.h"
#include "ypc/core_t/analyzer/var/data_hash_var.h"
#include "ypc/core_t/analyzer/var/enclave_hash_var.h"
#include "ypc/core_t/analyzer/var/encrypted_param_var.h"
#include "ypc/core_t/analyzer/var/intermediate_var.h"
#include "ypc/core_t/analyzer/var/request_key_var.h"
#include "ypc/core_t/analyzer/var/result_var.h"
#include "ypc/corecommon/crypto/group.h"
#include "ypc/corecommon/crypto/group_traits.h"
#include "ypc/corecommon/kgt.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {
template <typename Crypto>
class intermediate_result : virtual public request_key_var<true>,
                            virtual public enclave_hash_var,
                            virtual public result_var,
                            virtual public intermediate_var,
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
    // generate intermediate data hash and item number
    using ntt = ypc::nt<stbox::bytes>;
    auto pkg = ypc::make_package<ntt::batch_data_pkg_t>::from_bytes(
        result_var::m_result);
    crypto_t::hash_256(stbox::bytes("Fidelius"), m_intermediate_data_hash);
    for (auto &it : pkg.get<ntb::batch_data>()) {
      auto k = m_intermediate_data_hash + it;
      crypto_t::hash_256(k, m_intermediate_data_hash);
      m_item_number++;
    }

    stbox::bytes algo_skey_b, user_skey_b, dian_pkey;
    typename skey_group_t::key_t algo_skey, user_skey;

    // request for user key
    auto ret = keymgr_interface_t::request_private_key_for_public_key(
        m_user_pkey, user_skey_b, dian_pkey);

    // request for algo key
    ret = keymgr_interface_t::request_private_key_for_public_key(
        m_algo_pkey, algo_skey_b, dian_pkey);

    // construct skey generate tree for input data
    std::vector<std::shared_ptr<kgt<pkey_group_t>>> data_pkey_list;
    std::vector<std::shared_ptr<kgt<skey_group_t>>> data_skey_list;
    for (auto &data_kgt_pkey : m_data_kgt_pkey_list) {
      auto pkey_kgt_ptr = std::make_shared<kgt<pkey_group_t>>(data_kgt_pkey);
      data_pkey_list.push_back(pkey_kgt_ptr);
      std::unordered_map<stbox::bytes, stbox::bytes> peer;
      for (auto &l : pkey_kgt_ptr->leaves()) {
        stbox::bytes data_pkey_b((uint8_t *)&l->key_val, sizeof(l->key_val));
        stbox::bytes data_skey_b;
        ret = keymgr_interface_t::request_private_key_for_public_key(
            data_pkey_b, data_skey_b, dian_pkey);
        if (peer.find(data_pkey_b) == peer.end()) {
          peer.insert(std::make_pair(data_pkey_b, data_skey_b));
        }
      }
      auto skey_node = pkey_kgt_ptr->construct_skey_kgt_with_pkey_kgt(
          pkey_kgt_ptr->root(), peer);
      auto skey_kgt_ptr = std::make_shared<kgt<skey_group_t>>(skey_node);
      data_skey_list.push_back(skey_kgt_ptr);
    }

    // construct pkey generate tree for result
    typename pkey_group_t::key_t algo_pkey, user_pkey;
    memcpy(&algo_pkey, m_algo_pkey.data(), m_algo_pkey.size());
    memcpy(&user_pkey, m_user_pkey.data(), m_algo_pkey.size());
    auto algo_pkey_node =
        std::make_shared<ypc::key_node<pkey_group_t>>(algo_pkey);
    auto user_pkey_node =
        std::make_shared<ypc::key_node<pkey_group_t>>(user_pkey);
    std::vector<std::shared_ptr<ypc::key_node<pkey_group_t>>> children;
    children.push_back(algo_pkey_node);
    children.push_back(user_pkey_node);
    for (auto &pkey_kgt_ptr : data_pkey_list) {
      children.push_back(pkey_kgt_ptr->root());
    }
    auto pkey_node = std::make_shared<ypc::key_node<pkey_group_t>>();
    kgt<pkey_group_t> result_pkey_kgt(pkey_node, children);
    m_kgt_pkey = result_pkey_kgt.to_bytes();

    // calcualte skey generate tree for result
    memcpy(&algo_skey, algo_skey_b.data(), sizeof(algo_skey));
    memcpy(&user_skey, user_skey_b.data(), sizeof(user_skey));
    typename skey_group_t::key_t skey_sum;
    int add_ret = skey_group_t::add(skey_sum, algo_skey, user_skey);
    for (auto &skey_kgt_ptr : data_skey_list) {
      skey_kgt_ptr->calculate_kgt_sum();
      add_ret = skey_group_t::add(skey_sum, skey_sum, skey_kgt_ptr->sum());
    }

    // encrypt result
    stbox::bytes skey_sum_b = stbox::bytes(skey_sum.data, sizeof(skey_sum));
    stbox::bytes pkey_sum_b;
    ret = crypto_t::generate_pkey_from_skey(skey_sum_b, pkey_sum_b);
    ret = crypto_t::encrypt_message_with_prefix(
        pkey_sum_b, result_var::m_result, utc::crypto_prefix_arbitrary,
        m_encrypted_result_str);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << stbox::status_string(ret);
      return ret;
    }

    // sign all info
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
    ntt::intermediate_result_package_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::data_kgt_pkey>(m_kgt_pkey);
    pkg.set<ntt::intermediate_data_hash>(m_intermediate_data_hash);
    pkg.set<ntt::item_number>(m_item_number);
    ff::net::marshaler lm(ff::net::marshaler::length_retriver);
    pkg.arch(lm);
    return lm.get_length();
  }
  uint32_t get_analyze_result(uint8_t *result, uint32_t size) {
    using ntt = nt<stbox::bytes>;
    ntt::intermediate_result_package_t pkg;
    pkg.set<ntt::data_hash>(data_hash_var::m_data_hash);
    pkg.set<ntt::encrypted_result>(m_encrypted_result_str);
    pkg.set<ntt::result_signature>(m_result_signature_str);
    pkg.set<ntt::data_kgt_pkey>(m_kgt_pkey);
    pkg.set<ntt::intermediate_data_hash>(m_intermediate_data_hash);
    pkg.set<ntt::item_number>(m_item_number);
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
  stbox::bytes m_kgt_pkey;
  stbox::bytes m_intermediate_data_hash;
  int m_item_number;
};
} // namespace internal
template <typename Crypto>
using intermediate_result = internal::intermediate_result<Crypto>;

template <typename Crypto>
struct result_type_traits<intermediate_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::intermediate_result_parser;
};

} // namespace ypc
