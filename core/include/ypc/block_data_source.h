#pragma once
#include "corecommon/data_source.h"
#include "corecommon/nt_cols.h"
#include "glog/logging.h"
#include "ypc/sealed_file.h"
#include <ff/net/middleware/ntpackage.h>
#include <hpda/extractor/extractor_base.h>
#include <hpda/extractor/raw_data.h>

namespace ypc {
typedef ::ff::util::ntobject<nt<bytes>::data> data_source_output_t;

template <typename FT>
class block_data_source : public data_source<ypc::bytes> {
public:
  typedef data_source<ypc::bytes>::data_source_output_t user_item_t;
  block_data_source(FT *fh) : m_file(fh) { m_data_reach_end = false; }

  virtual ~block_data_source() {
    if (m_ret.data()) {
      m_ret.dealloc();
    }
  }

  virtual bool process() {
    if (m_data_reach_end) {
      return false;
    }
    if (m_ret.data()) {
      m_ret.dealloc();
    }
    m_data_reach_end = !(m_file->next_item(m_ret));
    return !m_data_reach_end;
  }

  virtual data_source_output_t output_value() {
    user_item_t ret;
    ret.set<nt<bytes>::data>(ypc::bytes(m_ret.data(), m_ret.size()));
    return ret;
  }

protected:
  FT *m_file;
  memref m_ret;
  bool m_data_reach_end;
};

} // namespace ypc
