#pragma once
#include "common/limits.h"
#include "hpda/extractor/extractor_base.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "ypc_t/ecommon/package.h"
#include <ff/util/ntobject.h>

namespace ypc {
using bytes = ::stbox::bytes;
typedef ::ff::util::ntobject<nt<bytes>::data> extra_data_source_output_t;

class extra_data_source : public ::hpda::extractor::internal::extractor_base<
                              extra_data_source_output_t> {
public:
  typedef extra_data_source_output_t user_item_t;

  extra_data_source(::stbox::dh_session_initiator *dh_session,
                    const stbox::bytes &data_hash);

  virtual ~extra_data_source();

  virtual bool process();

  virtual extra_data_source_output_t output_value();

  inline const bytes &data_hash() const { return m_data_hash; }

protected:
  ::stbox::dh_session_initiator *m_keymgr_session;

  bytes m_request_bytes;

  bytes m_data_hash;
  bytes m_req_data_hash;
  user_item_t m_data;
  // item_parser_t m_item_parser_func;

  sgx_package_handler m_phandler;
  bool m_data_reach_end;
  uint32_t m_counter;
};
} // namespace ypc
