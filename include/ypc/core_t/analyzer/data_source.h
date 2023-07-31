#pragma once
#include "hpda/extractor/extractor_base.h"
#include "ypc/common/limits.h"
#include "ypc/core_t/ecommon/package.h"
#include "ypc/corecommon/data_source.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#include "ypc/stbox/tsgx/channel/dh_session_initiator.h"
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

  inline void reset_reach_end() { m_data_reach_end = false; }

protected:
  bytes m_expect_data_hash;
  bool m_data_reach_end;
  uint32_t m_counter;
};

class data_source_with_merkle_hash : public data_source<bytes> {
public:
  data_source_with_merkle_hash(const stbox::bytes &data_hash)
      : m_expect_root_hash(data_hash) {}

  virtual ~data_source_with_merkle_hash() {}

  inline const std::vector<bytes> &expect_data_hash() const { return m_expect_data_hash; }
  virtual const std::vector<bytes> &data_hash() const = 0;

  inline void reset_reach_end() { m_data_reach_end = false; }

protected:
  bytes m_expect_root_hash; // 用于映射到数据源对应的文件上
  std::vector<bytes> m_expect_data_hash; // 用于验证完整性
  bool m_data_reach_end;
  uint32_t m_counter;
};
} // namespace ypc
