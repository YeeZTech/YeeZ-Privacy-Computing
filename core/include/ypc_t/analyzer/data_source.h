#pragma once
#include "common/limits.h"
#include "corecommon/data_source.h"
#include "hpda/extractor/extractor_base.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "ypc_t/ecommon/package.h"
#include <ff/util/ntobject.h>

namespace ypc {
using bytes = ::stbox::bytes;
typedef ::ff::util::ntobject<nt<bytes>::data> data_source_output_t;

class data_source_with_dhash : public data_source<bytes> {
public:
  inline data_source_with_dhash(const stbox::bytes &data_hash)
      : m_expect_data_hash(data_hash) {}

  virtual ~data_source_with_dhash() {}

  inline const bytes &expect_data_hash() const { return m_expect_data_hash; }
  virtual const bytes &data_hash() const = 0;

protected:
  bytes m_expect_data_hash;
  bool m_data_reach_end;
  uint32_t m_counter;
};
} // namespace ypc
