#pragma once
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/data_source.h"
#include "ypc_t/analyzer/internal/data_streams/multi_data_stream.h"
#include "ypc_t/analyzer/internal/data_streams/noinput_data_stream.h"

namespace ypc {
namespace internal {

template <typename DataSession> class data_source_var {
protected:
  std::shared_ptr<data_source_with_dhash> m_datasource;
  stbox::bytes m_ds_use_pkey;
};

template <> class data_source_var<multi_data_stream> {
protected:
  std::vector<std::shared_ptr<data_source_with_dhash>> m_datasource;
  std::vector<stbox::bytes> m_ds_use_pkey;
};
template <> class data_source_var<noinput_data_stream> {};
} // namespace internal
} // namespace ypc
