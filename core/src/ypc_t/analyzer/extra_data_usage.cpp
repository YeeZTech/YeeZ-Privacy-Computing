#include "common/crypto_prefix.h"
#include "common/limits.h"
#include "common/param_id.h"
#include "corecommon/package.h"
#include "stbox/ebyte.h"
#include "stbox/eth/util.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/log.h"
#include "yaenclave_t.h"
#include "ypc_t/analyzer/parser_wrapper_base.h"
#include "ypc_t/ecommon/signer_verify.h"
#include "ypc_t/ecommon/version.h"

namespace ypc {

using namespace stbox;
using ntt = nt<stbox::bytes>;

uint32_t parser_wrapper_base::set_extra_data(const uint8_t *extra_data,
                                             uint32_t in_size) {
  m_extra_data =
      make_package<ypc::nt<stbox::bytes>::extra_data_package_t>::from_bytes(
          extra_data, in_size);

  for (auto data_item : m_extra_data.get<ntt::extra_data_items>()) {
    const std::string &name = data_item.get<ntt::extra_data_group_name>();
    extra_data_source_group edsg;
    edsg.name = name;
    for (auto datahash : data_item.get<ntt::extra_data_hashes>()) {
    }
  }
  return stbox::stx_status::success;
}

uint32_t parser_wrapper_base::request_extra_data_usage() {

  for (auto data_item : m_extra_data.get<ntt::extra_data_items>()) {
    const std::string &name = data_item.get<ntt::extra_data_group_name>();
    extra_data_source_group edsg;
    edsg.name = name;
    for (auto datahash : data_item.get<ntt::extra_data_hashes>()) {
      stbox::bytes request_msg = make_bytes<stbox::bytes>::for_package<
          request_extra_data_usage_license_pkg_t, ntt::encrypted_param,
          ntt::pkey4v, ntt::data_hash>(m_encrypted_param, m_pkey4v, datahash);

      stbox::bytes recv;
      auto status = m_keymgr_session->send_request_recv_response(
          (char *)request_msg.data(), request_msg.size(),
          utc::max_keymgr_response_buf_size, recv);

      if (status != stbox::stx_status::success) {
        LOG(ERROR) << "error for m_keymgr_session->send_request_recv_response: "
                   << status;
        return status;
      }
      auto pkg =
          make_package<ack_extra_data_usage_license_pkg_t>::from_bytes(recv);
      auto succ = pkg.get<ntt::reserve>();
      if(succ){
        auto eds = std::make_shared<extra_data_source>(m_keymgr_session.get(),
                                                       datahash);
        eds->set_engine(&m_engine);
        edsg.data_sources.push_back(eds);
      }else{
        LOG(ERROR) << "invalid extra data usage license";
        return stbox::stx_status::extra_data_invalid_license;
      }
    }
    m_extra_data_sources.push_back(edsg);
  }
  return stbox::stx_status::success;
}
} // namespace ypc
