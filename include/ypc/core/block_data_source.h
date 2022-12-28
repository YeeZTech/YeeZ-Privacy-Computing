#pragma once
#include "ypc/core/sealed_file.h"
#include "ypc/corecommon/data_source.h"
#include "ypc/corecommon/nt_cols.h"
#include <exception>
#include <ff/net/middleware/ntpackage.h>
#include <glog/logging.h>
#include <hpda/extractor/extractor_base.h>
#include <hpda/extractor/raw_data.h>

namespace ypc {
using data_source_output_t = ::ff::util::ntobject<nt<bytes>::data>;

template <typename FT>
class block_data_source : public data_source<ypc::bytes> {
public:
  using user_item_t = data_source<ypc::bytes>::data_source_output_t;
  block_data_source(FT *fh) : m_file(fh) {
    m_data_reach_end = false;
    m_ret = std::unique_ptr<char[]>(new char[buf_size]);
  }

  virtual ~block_data_source() {
  }

  virtual bool process() {
    if (m_data_reach_end) {
      return false;
    }
    auto r = m_file->next_item(m_ret.get(), buf_size, m_ret_size);
    m_data_reach_end = (r == FT::eof);
    if (r != FT::succ && r != FT::eof) {
      LOG(ERROR) << "got unexpected error " << r;
      throw std::runtime_error("unexpected error while read file");
    }
    return !m_data_reach_end;
  }

  virtual data_source_output_t output_value() {
    user_item_t ret;
    ret.set<nt<bytes>::data>(ypc::bytes(m_ret.get(), m_ret_size));
    return ret;
  }

protected:
  constexpr static size_t buf_size = FT::BlockSizeLimit;
  FT *m_file;
  std::unique_ptr<char[]> m_ret;
  size_t m_ret_size;
  bool m_data_reach_end;
};

} // namespace ypc
