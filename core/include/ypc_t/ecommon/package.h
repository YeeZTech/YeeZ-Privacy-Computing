#pragma once
#include "stbox/ebyte.h"
#include "stbox/tsgx/log.h"
#include <ff/net/middleware/ntpackage.h>
#include <functional>
#include <unordered_map>

// namespace ypc {

template <uint32_t PackageID, typename... ARGS>
using sgx_package = ff::net::ntpackage<PackageID, ARGS...>;
using sgx_marshaler = ff::net::marshaler;

enum {
  request_data_item,
  ctrl_type,
  response_data_item,
};

define_nt(reserve, uint32_t);
define_nt(data, stbox::bytes);

typedef sgx_package<request_data_item, reserve> request_pkg_t;
typedef sgx_package<response_data_item, data> response_pkg_t;
typedef sgx_package<ctrl_type, reserve> ctrl_pkg_t;

class sgx_package_handler {
public:
  template <typename PkgType>
  void
  add_to_handle_pkg(const std::function<void(std::shared_ptr<PkgType>)> &f) {
    auto p = [f](const uint8_t *buf, size_t s) {
      sgx_marshaler m((const char *)buf, s, sgx_marshaler::deseralizer);
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

protected:
  typedef std::function<void(const uint8_t *buf, size_t s)> base_pkg_handler_t;
  std::unordered_map<uint32_t, base_pkg_handler_t> m_all_handlers;
};
//} // namespace ypc

