#include "ypc_t/analyzer/extra_data_source.h"
#include "corecommon/package.h"

namespace ypc {

extra_data_source::extra_data_source(::stbox::dh_session_initiator *dh_session,
                                     const stbox::bytes &data_hash)
    : m_keymgr_session(dh_session), m_req_data_hash(data_hash),
      m_data_reach_end(false), m_counter(0) {

  m_request_bytes =
      make_bytes<bytes>::for_package<request_extra_data_pkg_t,
                                     nt<bytes>::data_hash>(data_hash);

  // magic string here, Do Not Change!
  m_data_hash = stbox::eth::keccak256_hash(stbox::bytes("Fidelius"));

  m_phandler.add_to_handle_pkg<ack_extra_data_pkg_t>(
      [&](std::shared_ptr<ack_extra_data_pkg_t> p) {
        using data = typename nt<stbox::bytes>::data;
        const stbox::bytes &response = p->get<data>();
        if (response.size() == 0) {
          m_data_reach_end = true;
        } else {
          m_data_reach_end = false;
          stbox::bytes k = m_data_hash + response;
          m_data_hash = stbox::eth::keccak256_hash(k);
          m_data.set<nt<bytes>::data>(response);
        }
      });
  m_phandler.add_to_handle_pkg<ctrl_pkg_t>(
      [&](std::shared_ptr<ctrl_pkg_t> p) { m_data_reach_end = true; });
}

extra_data_source::~extra_data_source() {}

bool extra_data_source::process() {
  if (m_data_reach_end) {
    return false;
  }
  bytes recv;
  auto ret = m_keymgr_session->send_request_recv_response(
      (char *)m_request_bytes.data(), m_request_bytes.size(),
      ::ypc::utc::max_item_size, recv);
  if (ret != stbox::stx_status::success) {
    LOG(ERROR) << "error for m_datahub_session->send_request_recv_response: "
               << ret;
    m_data_reach_end = true;
    return false;
  }
  if (recv.size() == 0) {
    m_data_reach_end = true;
    return false;
  }

  m_phandler.handle_pkg(recv.data(), recv.size());
  m_counter++;
  return !m_data_reach_end;
}

extra_data_source_output_t extra_data_source::output_value() { return m_data; }
} // namespace ypc
