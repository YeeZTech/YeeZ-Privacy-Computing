#pragma once
#include "ypc/core_t/analyzer/interface/keymgr_interface.h"
#include "ypc/core_t/analyzer/internal/data_streams/multi_data_stream.h"
#include "ypc/core_t/analyzer/internal/data_streams/noinput_data_stream.h"
#include "ypc/core_t/analyzer/internal/data_streams/raw_data_stream.h"
#include "ypc/core_t/analyzer/internal/data_streams/sealed_data_stream.h"
#include "ypc/core_t/analyzer/internal/keymgr_session.h"
#include "ypc/core_t/analyzer/raw_data_provider.h"
#include "ypc/core_t/analyzer/sealed_data_provider.h"
#include "ypc/core_t/analyzer/var/data_source_var.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/corecommon/package.h"
#include "ypc/corecommon/kgt.h"
#include "ypc/corecommon/crypto/group.h"
#include "ypc/corecommon/crypto/group_traits.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_status.h"

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
  typedef Crypto crypto_t;
  typedef request_key_var<true> request_key_var_t;
  typedef typename ypc::crypto::group_traits<typename crypto_t::ecc_t>::group_t
      skey_group_t;
  typedef
      typename ypc::crypto::ecc_traits<skey_group_t>::peer_group_t pkey_group_t;

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
    stbox::bytes dian_pkey;

    LOG(INFO) << "pkg.get<ntt::pkey>(): "<< pkg.get<ntt::pkey>();
    kgt<pkey_group_t> pkey_kgt(pkg.get<ntt::pkey>());
    std::unordered_map<stbox::bytes, stbox::bytes> peer;
    for (auto &l : pkey_kgt.leaves()) {
      stbox::bytes data_pkey_b((uint8_t *)&l->key_val, sizeof(l->key_val));
      stbox::bytes data_skey_b;
      ret = keymgr_interface_t::request_private_key_for_public_key(
          data_pkey_b, data_skey_b, dian_pkey);
      if (peer.find(data_pkey_b) != peer.end()) {
        peer.insert(std::make_pair(data_pkey_b, data_skey_b));
      }
    }
    auto skey_node =
        pkey_kgt.construct_skey_kgt_with_pkey_kgt(pkey_kgt.root(), peer);
    kgt<skey_group_t> skey_kgt(skey_node);
    skey_kgt.calculate_kgt_sum();

    stbox::bytes pkey_kgt_sum((uint8_t *)&pkey_kgt.sum(), sizeof(pkey_kgt.sum()));
    stbox::bytes skey_kgt_sum((uint8_t *)&skey_kgt.sum(), sizeof(skey_kgt.sum()));
    m_ds_use_pkey = pkey_kgt_sum + pkg.get<ntt::data_hash>();

    m_datasource.reset(new sealed_data_provider<Crypto>(
        pkg.get<ntt::data_hash>(), skey_kgt_sum));
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
