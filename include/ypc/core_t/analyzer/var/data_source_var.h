#pragma once
#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/core_t/analyzer/internal/data_streams/intermediate_data_stream.h"
#include "ypc/core_t/analyzer/internal/data_streams/multi_data_stream.h"
#include "ypc/core_t/analyzer/internal/data_streams/noinput_data_stream.h"
#include "ypc/stbox/ebyte.h"

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
template <> class data_source_var<intermediate_data_stream> {
protected:
  std::vector<std::shared_ptr<data_source_with_dhash>> m_datasource;
  std::vector<stbox::bytes> m_ds_use_pkey;
};
} // namespace internal
} // namespace ypc
