#pragma once
#include "ypc/common/crypto_prefix.h"
#include "ypc/core_t/analyzer/interface/allowance_interface.h"
#include "ypc/core_t/analyzer/interface/do_parse_interface.h"
#include "ypc/core_t/analyzer/interface/forward_interface.h"
#include "ypc/core_t/analyzer/interface/keymgr_interface.h"
#include "ypc/core_t/analyzer/internal/is_param_encrypted.h"
#include "ypc/core_t/analyzer/internal/keymgr_session.h"
#include "ypc/core_t/analyzer/var/encrypted_param_var.h"
#include "ypc/core_t/analyzer/var/parser_var.h"
#include "ypc/core_t/analyzer/var/request_key_var.h"
#include "ypc/core_t/analyzer/var/result_var.h"
#include "ypc/core_t/analyzer/var/middata_var.h"
#include "ypc/corecommon/allowance.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {
typedef nt<stbox::bytes> ntt;

template <typename Crypto, typename DataSession, typename ParserT,
          typename Result, typename ModelT,
          template <typename> class DataAllowancePolicy,
          template <typename> class ModelAllowancePolicy,
          bool input_encrypted = is_param_encrypted<Result>::value>
class algo_interface {};

template <typename Crypto, typename DataSession, typename ParserT,
          typename Result, typename ModelT,
          template <typename> class DataAllowancePolicy,
          template <typename> class ModelAllowancePolicy>
class algo_interface<Crypto, DataSession, ParserT, Result, ModelT,
                     DataAllowancePolicy, ModelAllowancePolicy, true>
    : virtual public parser_var<ParserT>,
      virtual public request_key_var<true>,
      virtual public result_var,
      virtual public middata_var,
      virtual public do_parse_interface<ParserT, ModelT>,
      virtual public keymgr_interface<Crypto>,
      virtual public encrypted_param_var,
      virtual public check_allowance_interface<Crypto, ModelT, DataSession,
                                               DataAllowancePolicy,
                                               ModelAllowancePolicy>,
      virtual public forward_interface<Crypto, Result> {
  typedef Crypto ecc;
  typedef keymgr_interface<Crypto> keymgr_interface_t;
  typedef request_key_var<true> request_key_var_t;
  typedef do_parse_interface<ParserT, ModelT> do_parse_interface_t;
  typedef check_allowance_interface<Crypto, ModelT, DataSession,
                                    DataAllowancePolicy, ModelAllowancePolicy>
      allowance_checker_t;
  typedef forward_interface<Crypto, Result> forward_interface_t;

protected:
  uint32_t parse_data_item_impl(const uint8_t *input_param, uint32_t len) {
#ifdef DEBUG
    LOG(INFO) << "start parse data";
#endif
    ntt::param_t param =
        make_package<cast_obj_to_package<ntt::param_t>::type>::from_bytes(
            input_param, len);

#ifdef DEBUG
    LOG(INFO) << "done unmarshal param, start request private key";
#endif

    request_key_var_t::m_pkey4v = param.get<ntt::pkey>();
    middata_var::m_mid_pkey = param.get<ntt::pkey>();
    middata_var::m_algo_pkey = param.get<ntt::algo_pkey>();
    middata_var::m_data_kgt_pkey = param.get<ntt::data_kgt_pkey>();
    stbox::bytes dian_pkey;
    auto ret = keymgr_interface_t::request_private_key_for_public_key(
        request_key_var_t::m_pkey4v, request_key_var_t::m_private_key,
        dian_pkey);
    if (ret) {
      LOG(ERROR) << "request_private_key failed: " << stbox::status_string(ret);
      return ret;
    }
#ifdef DEBUG
    LOG(INFO) << "start decrypt param";
#endif
    stbox::bytes decrypted_param(
        ecc::get_decrypt_message_size_with_prefix(len));
    auto param_data = param.get<ntt::param_data>();

    ret = ecc::decrypt_message_with_prefix(request_key_var_t::m_private_key,
                                           param_data, decrypted_param,
                                           ypc::utc::crypto_prefix_arbitrary);
    if (ret) {
      LOG(ERROR) << "decrypt_message_with_prefix failed: "
                 << stbox::status_string(ret);
      return ret;
    }
#ifdef DEBUG
    LOG(INFO) << "start check allowance";
#endif
    param.set<ntt::param_data>(decrypted_param);
    auto tmp_allowance = param.get<ntt::allowances>();
    ret = allowance_checker_t::check_allowance(param);
    if (ret) {
      LOG(ERROR) << "check_allowance failed: " << stbox::status_string(ret);
      return ret;
    }

    ret = forward_interface_t::set_forward_target_info(param);
    if (ret) {
      LOG(ERROR) << "set_forward_target_info failed: "
                 << stbox::status_string(ret);
      return ret;
    }

    encrypted_param_var::m_encrypted_param = param_data;

#ifdef DEBUG
    LOG(INFO) << "start do_parse";
#endif
    result_var::m_result = do_parse_interface_t::do_parse(
        decrypted_param.data(), decrypted_param.size());
#ifdef DEBUG
    LOG(INFO) << "end parse ";
#endif
    result_var::m_cost_gas = 0;
    return stbox::stx_status::success;
  }
};

template <typename Crypto, typename DataSession, typename ParserT,
          typename Result, typename ModelT,
          template <typename> class DataAllowancePolicy,
          template <typename> class ModelAllowancePolicy>
class algo_interface<Crypto, DataSession, ParserT, Result, ModelT,
                     DataAllowancePolicy, ModelAllowancePolicy, false>
    : virtual public parser_var<ParserT>,
      virtual public result_var,
      virtual public keymgr_interface<Crypto>,
      virtual public do_parse_interface<ParserT, ModelT>,
      virtual public check_allowance_interface<Crypto, ModelT, DataSession,
                                               DataAllowancePolicy,
                                               ModelAllowancePolicy> {
  typedef do_parse_interface<ParserT, ModelT> do_parse_interface_t;
  typedef keymgr_interface<Crypto> keymgr_interface_t;
  typedef check_allowance_interface<Crypto, ModelT, DataSession,
                                    DataAllowancePolicy, ModelAllowancePolicy>
      allowance_checker_t;

protected:
  uint32_t parse_data_item_impl(const uint8_t *input_param, uint32_t len) {
#ifdef DEBUG
    LOG(INFO) << "start parse data";
#endif
    ntt::param_t param =
        make_package<cast_obj_to_package<ntt::param_t>::type>::from_bytes(
            input_param, len);

#ifdef DEBUG
    LOG(INFO) << "start check allowance";
#endif
    uint32_t ret = allowance_checker_t::check_allowance(param);
    if (ret) {
      LOG(ERROR) << "check_allowance failed: " << stbox::status_string(ret);
      return ret;
    }

#ifdef DEBUG
    LOG(INFO) << "start do_parse";
#endif
    auto param_data = param.get<ntt::param_data>();
    result_var::m_result =
        do_parse_interface_t::do_parse(param_data.data(), param_data.size());
#ifdef DEBUG
    LOG(INFO) << "end parse ";
#endif
    return stbox::stx_status::success;
  }
};
} // namespace internal
} // namespace ypc
