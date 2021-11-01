#pragma once
#include "common/limits.h"
#include "corecommon/package.h"
#include "hpda/extractor/extractor_base.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "stbox/tsgx/log.h"
#include "ypc_t/ecommon/package.h"
#include <ff/util/ntobject.h>

namespace ypc {
using bytes = ::stbox::bytes;
template <typename OutputObjType>
class sealed_data_provider
    : public ::hpda::extractor::internal::extractor_base<OutputObjType> {
public:
  typedef OutputObjType user_item_t;
  typedef user_item_t (*item_parser_t)(const stbox::bytes::byte_t *, size_t);

  sealed_data_provider(::stbox::dh_session_initiator *dh_session)
      : m_datahub_session(dh_session) {
    m_request_bytes =
        make_bytes<bytes>::for_package<request_pkg_t,
                                       ypc::nt<stbox::bytes>::reserve>(0);

    // magic string here, Do Not Change!
    m_data_hash = stbox::eth::keccak256_hash(stbox::bytes("Fidelius"));

    m_phandler.add_to_handle_pkg<response_pkg_t>(
        [&](std::shared_ptr<response_pkg_t> p) {
          using data = typename nt<stbox::bytes>::data;
          const stbox::bytes &response = p->get<data>();
          stbox::bytes k = m_data_hash + response;
          m_data_hash = stbox::eth::keccak256_hash(k);
          m_data = m_item_parser_func(response.data(), response.size());
        });
    m_phandler.add_to_handle_pkg<ctrl_pkg_t>(
        [&](std::shared_ptr<ctrl_pkg_t> p) { m_data_reach_end = true; });
  }

  inline void set_item_parser(item_parser_t func) { m_item_parser_func = func; }

  virtual ~sealed_data_provider() {}

  virtual bool process() {
    if (m_data_reach_end) {
      return false;
    }
    bytes recv;
    LOG(INFO) << "send request";
    auto ret = m_datahub_session->send_request_recv_response(
        (char *)m_request_bytes.data(), m_request_bytes.size(),
        ::ypc::utc::max_item_size, recv);
    LOG(INFO) << "send request and got " << ret;
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "error for m_datahub_session->send_request_recv_response: "
                 << ret;
      return false;
    }
    m_phandler.handle_pkg(recv.data(), recv.size());
    return true;
  }

  virtual OutputObjType output_value() { return m_data; }

  const bytes &data_hash() const { return m_data_hash; }

protected:
  ::stbox::dh_session_initiator *m_datahub_session;

  bytes m_request_bytes;

  bytes m_data_hash;
  user_item_t m_data;
  item_parser_t m_item_parser_func;

  sgx_package_handler m_phandler;
  bool m_data_reach_end;
};
} // namespace ypc
