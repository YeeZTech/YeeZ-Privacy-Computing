#pragma once
#include "corecommon/nt_cols.h"
#include "hpda/extractor/extractor_base.h"
#include <ff/util/ntobject.h>

namespace ypc {
template <class BytesType>
class data_source : public ::hpda::extractor::internal::extractor_base<
                        ::ff::util::ntobject<typename nt<BytesType>::data>> {
public:
  typedef ::ff::util::ntobject<typename nt<BytesType>::data>
      data_source_output_t;
  virtual ~data_source() {}
};
} // namespace ypc
