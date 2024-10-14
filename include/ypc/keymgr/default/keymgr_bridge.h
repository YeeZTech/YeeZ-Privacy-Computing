#pragma once
#include "ypc/keymgr/default/keymgr_sgx_module.h"

namespace ypc {
class keymgr_parser {
public:
  keymgr_parser(std::shared_ptr<ypc::keymgr_sgx_module> &keymgr)
      : m_keymgr(keymgr) {}

  inline std::shared_ptr<ypc::keymgr_sgx_module> &keymgr() { return m_keymgr; }

protected:
  std::shared_ptr<ypc::keymgr_sgx_module> m_keymgr;
};
void init_sgx_keymgr(std::shared_ptr<ypc::keymgr_sgx_module> &keymgr);
void shutdown_sgx_keymgr();
} // namespace ypc
