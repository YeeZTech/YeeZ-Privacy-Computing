#pragma once
#include "corecommon/allowance.h"
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/data_source.h"
#include "ypc_t/analyzer/interface/keymgr_interface.h"
#include "ypc_t/analyzer/internal/data_streams/multi_data_stream.h"
#include "ypc_t/analyzer/internal/data_streams/noinput_data_stream.h"
#include "ypc_t/analyzer/internal/data_streams/raw_data_stream.h"
#include "ypc_t/analyzer/internal/data_streams/sealed_data_stream.h"
#include "ypc_t/analyzer/var/data_source_var.h"
#include "ypc_t/analyzer/var/enclave_hash_var.h"
#include "ypc_t/analyzer/var/model_var.h"

namespace ypc {
template <typename ModelT>
struct check_model_allowance : virtual public internal::model_var<ModelT> {

  typedef internal::model_var<ModelT> model_var_t;
  uint32_t check_allowance_m(const std::vector<stbox::bytes> &checked_pkey) {
    for (auto pkey : checked_pkey) {
      if (pkey == model_var_t::m_model_pkey + model_var_t::m_model_hash) {
        return stbox::stx_status::success;
      }
    }
    return stbox::stx_status::model_allowance_miss;
  }
};
template <> struct check_model_allowance<void> {
  uint32_t check_allowance_m(const std::vector<stbox::bytes> &checked_pkey) {
    return stbox::stx_status::success;
  }
};

template <typename ModelT> struct ignore_model_allowance {
  uint32_t check_allowance_m(const std::vector<stbox::bytes> &checked_pkey) {
    return stbox::stx_status::success;
  }
};

template <typename DataSession> struct check_data_allowance {
  uint32_t check_allowance_d(const std::vector<stbox::bytes> &checked_pkey) {
    return stbox::stx_status::success;
  }
};
template <>
struct check_data_allowance<sealed_data_stream>
    : virtual public internal::data_source_var<sealed_data_stream> {
  uint32_t check_allowance_d(const std::vector<stbox::bytes> &checked_pkey) {
    typedef internal::data_source_var<sealed_data_stream> data_source_var_t;
    stbox::bytes pkey = data_source_var_t::m_ds_use_pkey;
    for (auto pk : checked_pkey) {
      if (pk == pkey) {
        return stbox::stx_status::success;
      }
    }
    LOG(ERROR) << "data source "
               << data_source_var_t::m_datasource->expect_data_hash()
               << " have no allowance";
    return stbox::stx_status::data_allowance_miss;
  }
};
template <>
struct check_data_allowance<multi_data_stream>
    : virtual public internal::data_source_var<multi_data_stream> {
  uint32_t check_allowance_d(const std::vector<stbox::bytes> &checked_pkey) {
    for (auto dsp : data_source_var<multi_data_stream>::m_ds_use_pkey) {
      bool used = false;
      for (auto pk : checked_pkey) {
        if (pk == dsp) {
          used = true;
          break;
        }
      }
      if (!used) {
        LOG(ERROR) << "cannot find allowance to access data with pkey: " << dsp;
        return stbox::stx_status::data_allowance_miss;
      }
    }
    return stbox::stx_status::success;
  }
};

template <typename DataSession> struct ignore_data_allowance {
  uint32_t check_allowance_d(const std::vector<stbox::bytes> &checked_pkey) {
    return stbox::stx_status::success;
  }
};

namespace internal {

template <typename Crypto, typename ModelT, typename DataSession,
          template <typename> class DataAllowancePolicy,
          template <typename> class ModelAllowancePolicy>
struct check_allowance_interface
    : virtual public ModelAllowancePolicy<ModelT>,
      virtual public DataAllowancePolicy<DataSession>,
      virtual public keymgr_interface<Crypto>,
      virtual public enclave_hash_var {
  typedef keymgr_interface<Crypto> keymgr_interface_t;
  typedef nt<stbox::bytes> ntt;
  typedef ModelAllowancePolicy<ModelT> model_checker_t;
  typedef DataAllowancePolicy<DataSession> data_checker_t;

public:
  uint32_t check_allowance(const ntt::param_t &param) {
    auto param_data = param.get<ntt::param_data>();
    stbox::bytes param_hash;
    auto ret = Crypto::sha3_256(param_data, param_hash);
    if (ret) {
      LOG(ERROR) << "sha3_256 failed: " << stbox::status_string(ret);
      return ret;
    }

    std::vector<stbox::bytes> checked_pkey;
    auto allowances = param.get<ntt::allowances>();
    for (auto allowance_i : allowances) {
      stbox::bytes pkey4v = allowance_i.get<ntt::pkey>();
      if (pkey4v.size() != 0) {
        stbox::bytes private_key;
        stbox::bytes dian_pkey;
        ret = keymgr_interface_t::request_private_key_for_public_key(
            pkey4v, private_key, dian_pkey);
        if (ret) {
          LOG(ERROR) << "request_private_key failed: "
                     << stbox::status_string(ret);
          return ret;
        }

        stbox::bytes allow = allowance_i.get<ntt::signature>();

        stbox::bytes to_check_data =
            param_hash + enclave_hash_var::m_enclave_hash + dian_pkey +
            allowance_i.get<ntt::data_hash>();

        ret = allowance<Crypto>::check(private_key, to_check_data, allow);
        if (ret) {
          LOG(WARNING) << "check_allowance" << allow
                       << " failed: " << stbox::status_string(ret)
                       << ", ignore it";
        } else {
          checked_pkey.push_back(pkey4v + allowance_i.get<ntt::data_hash>());
        }
      }
    }

    ret = model_checker_t::check_allowance_m(checked_pkey);
    if (ret) {
      LOG(ERROR) << "check_model_allowance failed: "
                 << stbox::status_string(ret);
      return ret;
    }
    ret = data_checker_t::check_allowance_d(checked_pkey);
    if (ret) {
      LOG(ERROR) << "check_data_allowance failed: "
                 << stbox::status_string(ret);
      return ret;
    }
    return ret;
  }
};

} // namespace internal
} // namespace ypc
