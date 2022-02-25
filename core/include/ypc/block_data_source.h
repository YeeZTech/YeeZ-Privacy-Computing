#pragma once
#include "glog/logging.h"
#include "ypc/sealed_file.h"
#include <ff/net/middleware/ntpackage.h>
#include <hpda/extractor/extractor_base.h>
#include <hpda/extractor/raw_data.h>

namespace ypc {

template <typename OutputObjType, typename FT>
class block_data_source
    : public ::hpda::extractor::internal::extractor_base<OutputObjType> {
public:
  typedef OutputObjType user_item_t;
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

  virtual OutputObjType output_value() {
    user_item_t ret;
    ff::net::marshaler m((char *)m_ret.data(), m_ret.size(),
                         ff::net::marshaler::deserializer);
    ret.arch(m);
    return ret;
  }

protected:
  FT *m_file;
  bool m_data_reach_end;
  memref m_ret;
};

} // namespace ypc
