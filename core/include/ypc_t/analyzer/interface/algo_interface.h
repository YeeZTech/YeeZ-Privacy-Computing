#pragma once
#include "common/crypto_prefix.h"
#include "corecommon/allowance.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/interface/do_parse_interface.h"
#include "ypc_t/analyzer/interface/keymgr_interface.h"
#include "ypc_t/analyzer/internal/is_param_encrypted.h"
#include "ypc_t/analyzer/internal/keymgr_session.h"
#include "ypc_t/analyzer/var/encrypted_param_var.h"
#include "ypc_t/analyzer/var/parser_var.h"
#include "ypc_t/analyzer/var/request_key_var.h"
#include "ypc_t/analyzer/var/result_var.h"

namespace ypc {
namespace internal {
typedef nt<stbox::bytes> ntt;

template <typename Crypto, typename ParserT, typename Result, typename ModelT,
          bool input_encrypted = is_param_encrypted<Result>::value>
class algo_interface {};

template <typename Crypto, typename ParserT, typename Result, typename ModelT>
class algo_interface<Crypto, ParserT, Result, ModelT, true>
    : virtual public parser_var<ParserT>,
      virtual public request_key_var<true>,
      virtual public result_var,
      virtual public do_parse_interface<ParserT, ModelT>,
      virtual public keymgr_interface<Crypto>,
      virtual public encrypted_param_var {
  typedef Crypto ecc;
  typedef keymgr_interface<Crypto> keymgr_interface_t;
  typedef request_key_var<true> request_key_var_t;
  typedef do_parse_interface<ParserT, ModelT> do_parse_interface_t;

protected:
  uint32_t parse_data_item_impl(const uint8_t *input_param, uint32_t len) {
    // TODO allowance may include data usage license for hosting data
    ntt::param_t param =
        make_package<cast_obj_to_package<ntt::param_t>::type>::from_bytes(
            input_param, len);

    request_key_var_t::m_pkey4v = param.get<ntt::allowance>().get<ntt::pkey>();
    auto ret = keymgr_interface_t::request_private_key_for_public_key(
        request_key_var_t::m_pkey4v, request_key_var_t::m_private_key);
    if (ret) {
      LOG(ERROR) << "request_private_key failed: " << stbox::status_string(ret);
      return ret;
    }
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
    encrypted_param_var::m_encrypted_param = stbox::bytes(input_param, len);
    result_var::m_result = do_parse_interface_t::do_parse(
        decrypted_param.data(), decrypted_param.size());
    result_var::m_cost_gas = 0;
    return stbox::stx_status::success;
  }
};

template <typename Crypto, typename ParserT, typename Result, typename ModelT>
class algo_interface<Crypto, ParserT, Result, ModelT, false>
    : virtual public parser_var<ParserT>,
      virtual public result_var,
      virtual public keymgr_interface<Crypto>,
      virtual public do_parse_interface<ParserT, ModelT> {
  typedef do_parse_interface<ParserT, ModelT> do_parse_interface_t;
  typedef keymgr_interface<Crypto> keymgr_interface_t;

protected:
  uint32_t parse_data_item_impl(const uint8_t *input_param, uint32_t len) {
    ntt::param_t param =
        make_package<cast_obj_to_package<ntt::param_t>::type>::from_bytes(
            input_param, len);

    stbox::bytes pkey4v = param.get<ntt::allowance>().get<ntt::pkey>();
    stbox::bytes private_key;
    auto ret = keymgr_interface_t::request_private_key_for_public_key(
        pkey4v, private_key);
    if (ret) {
      LOG(ERROR) << "request_private_key failed: " << stbox::status_string(ret);
      return ret;
    }

    stbox::bytes allow = param.get<ntt::allowance>().get<ntt::encrypted_sig>();

    auto param_data = param.get<ntt::param_data>();
    stbox::bytes param_hash;
    ret = Crypto::sha3_256(param_data, param_hash);
    if (ret) {
      LOG(ERROR) << "sha3_256 failed: " << stbox::status_string(ret);
      return ret;
    }
    ret = allowance<Crypto>::check(private_key, param_hash, allow);

    if (ret) {
      LOG(ERROR) << "check_allowance failed: " << stbox::status_string(ret);
      return ret;
    }

    result_var::m_result =
        do_parse_interface_t::do_parse(param_data.data(), param_data.size());
    return stbox::stx_status::success;
  }
};
} // namespace internal
} // namespace ypc
