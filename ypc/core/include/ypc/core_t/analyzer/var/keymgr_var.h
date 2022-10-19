#pragma once
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/tsgx/channel/dh_session_initiator.h"

namespace ypc {
namespace internal {

class keymgr_var : virtual public enclave_hash_var {
protected:
  std::unique_ptr<::stbox::dh_session_initiator> m_keymgr_session;
};
} // namespace internal
} // namespace ypc
