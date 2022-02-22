#pragma once
#include "common/crypto_prefix.h"
#include "corecommon/allowance.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/interface/keymgr_interface.h"
#include "ypc_t/analyzer/internal/results/forward_result.h"

namespace ypc {
namespace internal {
template <typename Crypto, typename Result> class forward_interface {
public:
  typedef nt<stbox::bytes> ntt;
  uint32_t set_forward_target_info(const ntt::param_t &param) {
    return stbox::stx_status::success;
  }
};

template <typename Crypto>
class forward_interface<Crypto, forward_result<Crypto>>
    : virtual public forward_result<Crypto>,
      virtual public enclave_hash_var,
      virtual public keymgr_interface<Crypto> {
  typedef forward_result<Crypto> forward_result_t;
  typedef keymgr_interface<Crypto> keymgr_interface_t;

public:
  typedef nt<stbox::bytes> ntt;
  uint32_t set_forward_target_info(const ntt::param_t &param) {
    auto param_data = param.get<ntt::param_data>();

    stbox::bytes pkey4v = param.get<ntt::pkey>();
    stbox::bytes private_key;
    stbox::bytes dian_pkey;
    auto ret = keymgr_interface_t::request_private_key_for_public_key(
        pkey4v, private_key, dian_pkey);
    if (ret) {
      LOG(ERROR) << "request_private_key failed: " << stbox::status_string(ret);
      return ret;
    }


    auto forward_info = param.get<ntt::forward>();

    auto to_check_data = param_data + enclave_hash_var::m_enclave_hash +
                         dian_pkey + forward_info.get<ntt::enclave_hash>() +
                         forward_info.get<ntt::pkey>();
    auto allow = forward_info.get<ntt::encrypted_sig>();
    ret = allowance<Crypto>::check(private_key, to_check_data, allow);
    if (ret) {
      LOG(WARNING) << "check_allowance:" << allow
                   << " failed: " << stbox::status_string(ret) << ", ignore it";
      return stbox::stx_status::forward_allowance_invalid;
    }

    forward_result_t::m_target_enclave_hash =
        forward_info.get<ntt::enclave_hash>();
    forward_result_t::m_target_dian_pkey = forward_info.get<ntt::pkey>();

    return stbox::stx_status::success;
  }
};
} // namespace internal
} // namespace ypc
