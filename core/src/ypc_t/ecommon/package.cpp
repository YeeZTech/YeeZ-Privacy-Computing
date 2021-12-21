#include "ypc_t/ecommon/package.h"
#include "stbox/stx_common.h"

// namespace ypc {
void sgx_package_handler::handle_pkg(const uint8_t *buf, size_t len) {
  uint32_t type_id;
  ::ff::net::deserialize((const char *)buf, type_id);
  auto it = m_all_handlers.find(type_id);
  if (it == m_all_handlers.end()) {
    throw std::runtime_error("no handler for type id");
  }
  it->second(buf, len);
}
//} // namespace ypc
