#pragma once
#include "stbox/ebyte.h"
#include "stbox/tsgx/log.h"
#include <corecommon/nt_cols.h>
#include <ff/net/middleware/ntpackage.h>
#include <functional>
#include <unordered_map>

// namespace ypc {

template <uint32_t PackageID, typename... ARGS>
using sgx_package = ff::net::ntpackage<PackageID, ARGS...>;
using sgx_marshaler = ff::net::marshaler;

enum {
  request_data_item = 1,
  ctrl_type,
  response_data_item,
  request_private_key_item,
  request_skey_from_pkey_item,
  request_extra_data_usage_license_item,
  ack_extra_data_usage_license_item,
  request_extra_data_item,
  ack_extra_data_item
};

typedef sgx_package<request_data_item, ypc::nt<stbox::bytes>::reserve>
    request_pkg_t;
typedef sgx_package<response_data_item, ypc::nt<stbox::bytes>::data>
    response_pkg_t;
typedef sgx_package<ctrl_type, ypc::nt<stbox::bytes>::reserve> ctrl_pkg_t;
typedef sgx_package<request_private_key_item, ypc::nt<stbox::bytes>::id>
    request_private_key_pkg_t;
typedef sgx_package<request_skey_from_pkey_item, ypc::nt<stbox::bytes>::pkey>
    request_skey_from_pkey_pkg_t;

typedef sgx_package<request_extra_data_usage_license_item,
                    ypc::nt<stbox::bytes>::encrypted_param,
                    ypc::nt<stbox::bytes>::pkey4v,
                    ypc::nt<stbox::bytes>::data_hash>
    request_extra_data_usage_license_pkg_t;
typedef sgx_package<ack_extra_data_usage_license_item,
                    ypc::nt<stbox::bytes>::reserve>
    ack_extra_data_usage_license_pkg_t;

typedef sgx_package<request_extra_data_item, ypc::nt<stbox::bytes>::data_hash>
    request_extra_data_pkg_t;
typedef sgx_package<ack_extra_data_item, ypc::nt<stbox::bytes>::data>
    ack_extra_data_pkg_t;

class sgx_package_handler {
public:
  template <typename PkgType>
  void
  add_to_handle_pkg(const std::function<void(std::shared_ptr<PkgType>)> &f) {
    auto p = [f](const uint8_t *buf, size_t s) {
      sgx_marshaler m((const char *)buf, s, sgx_marshaler::deserializer);
      std::shared_ptr<PkgType> pkg(new PkgType());
      pkg->arch(m);
      f(pkg);
    };
    m_all_handlers.insert(std::make_pair(PkgType::package_id, p));
  }

  //(TODO)add_low_level_handler for memory optimization

  template <typename T> void handle_pkg(const T *buf, size_t len) {
    handle_pkg((const uint8_t *)buf, len);
  }
  void handle_pkg(const uint8_t *buf, size_t len);

  template <typename PkgType> bool is_pkg_handled() const {
    return m_all_handlers.find(PkgType::package_id) != m_all_handlers.end();
  }

protected:
  typedef std::function<void(const uint8_t *buf, size_t s)> base_pkg_handler_t;
  std::unordered_map<uint32_t, base_pkg_handler_t> m_all_handlers;
};
//} // namespace ypc

