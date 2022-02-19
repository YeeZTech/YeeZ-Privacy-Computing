#pragma once
#include "corecommon/nt_cols.h"
#include "corecommon/package.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/internal/keymgr_session.h"
#include "ypc_t/analyzer/var/enclave_hash_var.h"
#include "ypc_t/analyzer/var/keymgr_var.h"
#include "ypc_t/analyzer/var/model_var.h"
#include "ypc_t/analyzer/var/request_key_var.h"

namespace ypc {
namespace internal {
typedef nt<stbox::bytes> ntt;

template <typename Crypto, typename ModelT,
          bool has_model = !std::is_same<ModelT, void>::value>
class model_interface {};

template <typename Crypto, typename ModelT>
class model_interface<Crypto, ModelT, true>
    : virtual public model_var<ModelT>,
      virtual public enclave_hash_var,
      virtual public internal::keymgr_session,
      virtual public keymgr_interface<Crypto>,
      virtual public request_key_var<true> {
  typedef Crypto ecc;
  typedef keymgr_interface<Crypto> keymgr_interface_t;
  typedef request_key_var<true> request_key_var_t;
  typedef model_var<ModelT> model_var_t;

public:
  uint32_t init_model(const uint8_t *model, uint32_t len) {
    auto ret = internal::keymgr_session::init_keymgr_session();
    if (ret) {
      LOG(ERROR) << "init_keymgr_session failed: " << stbox::status_string(ret);
      return ret;
    }
    ntt::model_t mod =
        make_package<cast_obj_to_package<ntt::model_t>::type>::from_bytes(model,
                                                                          len);

    stbox::bytes private_key, dian_pkey;
    ret = keymgr_interface_t::request_private_key_for_public_key(
        mod.get<ntt::pkey>(), private_key, dian_pkey);
    if (ret) {
      LOG(ERROR) << "request_private_key failed: " << stbox::status_string(ret);
      return ret;
    }


    auto model_data = mod.get<ntt::model_data>();
    stbox::bytes decrypted_model(
        ecc::get_decrypt_message_size_with_prefix(model_data.size()));

    ret = ecc::decrypt_message_with_prefix(
        private_key.data(), private_key.size(), model_data.data(),
        model_data.size(), decrypted_model.data(), decrypted_model.size(),
        ypc::utc::crypto_prefix_arbitrary);
    if (ret) {
      LOG(ERROR) << "decrypt_message_with_prefix failed: "
                 << stbox::status_string(ret);
      return ret;
    }

    model_var_t::m_model =
        make_package<typename cast_obj_to_package<ModelT>::type>::from_bytes(
            decrypted_model);
    model_var_t::m_model_pkey = mod.get<ntt::pkey>();
    ret = ecc::sha3_256(model_data, model_var_t::m_model_hash);
    if (ret) {
      LOG(ERROR) << "sha3_256 failed: " << stbox::status_string(ret);
      return ret;
    }

    return stbox::stx_status::success;
  }
};
} // namespace internal
} // namespace ypc
