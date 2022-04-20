#pragma oncew
#include "corecommon/nt_cols.h"
#include "corecommon/package.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/interface/keymgr_interface.h"
#include "ypc_t/analyzer/internal/data_streams/multi_data_stream.h"
#include "ypc_t/analyzer/internal/data_streams/noinput_data_stream.h"
#include "ypc_t/analyzer/internal/data_streams/raw_data_stream.h"
#include "ypc_t/analyzer/internal/data_streams/sealed_data_stream.h"
#include "ypc_t/analyzer/internal/keymgr_session.h"
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
  uint32_t check_actual_data_hash() {
    auto p = m_datasource.get();
    if (p->data_hash() != p->expect_data_hash()) {
      LOG(ERROR) << "expect " << p->expect_data_hash() << ", got "
                 << p->data_hash();
      return stbox::stx_status::data_hash_not_same_as_expect;
    }
    return stbox::stx_status::success;
  }
};

template <typename Crypto>
class data_interface<Crypto, noinput_data_stream>
    : virtual public data_source_var<noinput_data_stream> {
public:
  uint32_t check_actual_data_hash() { return stbox::stx_status::success; }
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
    auto pkg = make_package<typename cast_obj_to_package<
        ntt::sealed_data_info_t>::type>::from_bytes(data_source_info, len);
    auto ret = keymgr_session::init_keymgr_session();
    if (ret) {
      LOG(ERROR) << "init_keymgr_session failed: " << stbox::status_string(ret);
      return ret;
    }
    stbox::bytes private_key, dian_pkey;
    ret = keymgr_interface_t::request_private_key_for_public_key(
        pkg.get<ntt::pkey>(), private_key, dian_pkey);
    if (ret) {
      LOG(ERROR) << "request_private_key_for_public_key failed: "
                 << stbox::status_string(ret);
      return ret;
    }

    m_ds_use_pkey = pkg.get<ntt::pkey>() + pkg.get<ntt::data_hash>();
    m_datasource.reset(new sealed_data_provider<Crypto>(
        pkg.get<ntt::data_hash>(), private_key));
    return stbox::stx_status::success;
  }

  uint32_t check_actual_data_hash() {
    auto p = m_datasource.get();
    if (p->data_hash() != p->expect_data_hash()) {
      LOG(ERROR) << "expect " << p->expect_data_hash() << ", got "
                 << p->data_hash();
      return stbox::stx_status::data_hash_not_same_as_expect;
    }
    return stbox::stx_status::success;
  }
};

template <typename Crypto>
class data_interface<Crypto, multi_data_stream>
    : virtual public data_source_var<multi_data_stream>,
      virtual public keymgr_session,
      virtual public keymgr_interface<Crypto> {
  typedef keymgr_interface<Crypto> keymgr_interface_t;

public:
  uint32_t init_data_source(const uint8_t *data_source_info, uint32_t len) {
    using ntt = nt<stbox::bytes>;
    auto pkg = make_package<typename cast_obj_to_package<
        ntt::multi_sealed_data_info_t>::type>::from_bytes(data_source_info,
                                                          len);
    auto ret = keymgr_session::init_keymgr_session();
    if (ret) {
      LOG(ERROR) << "init_keymgr_session failed: " << stbox::status_string(ret);
      return ret;
    }
    auto infos = pkg.get<ntt::sealed_data_info_vector>();
    for (auto sdi : infos) {
      if (sdi.get<ntt::pkey>().size() != 0) {
        stbox::bytes private_key, dian_pkey;
        ret = keymgr_interface_t::request_private_key_for_public_key(
            sdi.get<ntt::pkey>(), private_key, dian_pkey);
        auto hash = sdi.get<ntt::data_hash>();
        if (ret) {
          LOG(ERROR) << "request_private_key_for_public_key for data hash "
                     << hash << " failed: " << stbox::status_string(ret);
          return ret;
        }
        m_ds_use_pkey.push_back(sdi.get<ntt::pkey>() +
                                sdi.get<ntt::data_hash>());
        m_datasource.push_back(std::shared_ptr<data_source_with_dhash>(
            new sealed_data_provider<Crypto>(sdi.get<ntt::data_hash>(),
                                             private_key)));
      } else {
        m_datasource.push_back(std::shared_ptr<data_source_with_dhash>(
            new raw_data_provider(sdi.get<ntt::data_hash>())));
      }
    }
    return stbox::stx_status::success;
  }

  uint32_t check_actual_data_hash() {
    for (auto ds : m_datasource) {
      if (ds->data_hash() != ds->expect_data_hash()) {
        LOG(ERROR) << "expect " << ds->expect_data_hash() << ", got "
                   << ds->data_hash();
        return stbox::stx_status::data_hash_not_same_as_expect;
      }
    }
    return stbox::stx_status::success;
  }
};
} // namespace internal
} // namespace ypc
