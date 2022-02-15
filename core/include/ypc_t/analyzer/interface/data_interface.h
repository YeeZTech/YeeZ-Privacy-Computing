#pragma once
#include "corecommon/nt_cols.h"
#include "corecommon/package.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/internal/keymgr_session.h"
#include "ypc_t/analyzer/internal/multi_data_stream.h"
#include "ypc_t/analyzer/internal/noinput_data_stream.h"
#include "ypc_t/analyzer/internal/raw_data_stream.h"
#include "ypc_t/analyzer/internal/sealed_data_stream.h"
#include "ypc_t/analyzer/raw_data_provider.h"
#include "ypc_t/analyzer/sealed_data_provider.h"
#include "ypc_t/analyzer/var/data_source_var.h"

namespace ypc {
namespace internal {

template <typename Crypto, typename DataSession> class data_interface {};

template <typename Crypto>
class data_interface<Crypto, raw_data_stream>
    : virtual public data_source_var<raw_data_stream> {
public:
  uint32_t init_data_source(const uint8_t *data_source_info, uint32_t len) {
    // in this case, @data_source_info is data hash
    m_datasource.reset(
        new raw_data_provider(stbox::bytes(data_source_info, len)));
    return stbox::stx_status::success;
  }
};

template <typename Crypto>
class data_interface<Crypto, noinput_data_stream>
    : virtual public data_source_var<noinput_data_stream> {
};

template <typename Crypto>
class data_interface<Crypto, sealed_data_stream>
    : virtual public data_source_var<sealed_data_stream>,
      virtual public keymgr_interface<Crypto>,
      virtual public keymgr_session {
  typedef keymgr_interface<Crypto> keymgr_interface_t;

public:
  uint32_t init_data_source(const uint8_t *data_source_info, uint32_t len) {
    using ntt = nt<stbox::bytes>;
    auto pkg = make_package<ntt::sealed_data_info_pkg_t>::from_bytes(
        data_source_info, len);
    auto ret = keymgr_session::init_keymgr_session();
    if (ret) {
      LOG(ERROR) << "init_keymgr_session failed: " << stbox::status_string(ret);
      return ret;
    }
    stbox::bytes private_key;
    ret = keymgr_interface_t::request_private_key_for_public_key(
        pkg.get<ntt::pkey>(), private_key);
    if (ret) {
      LOG(ERROR) << "request_private_key_for_public_key failed: "
                 << stbox::status_string(ret);
      return ret;
    }

    m_datasource.reset(new sealed_data_provider<Crypto>(
        pkg.get<ntt::data_hash>(), private_key));
    return stbox::stx_status::success;
  }
};

template <typename Crypto>
class data_interface<Crypto, multi_data_stream>
    : virtual public data_source_var<multi_data_stream>,
      virtual public keymgr_session {
public:
  uint32_t init_data_source(const uint8_t *data_source_info, uint32_t len) {
    //<data_hash, pkey>
    // request_private_key
    // m_datasource.reset(new raw_data_provider(data_hash, private_key));
    return 1;
  }
};
} // namespace internal
} // namespace ypc
