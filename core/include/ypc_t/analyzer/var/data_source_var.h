#pragma once
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/data_source.h"
#include "ypc_t/analyzer/internal/multi_data_stream.h"
#include "ypc_t/analyzer/internal/noinput_data_stream.h"

namespace ypc {
namespace internal {

template <typename DataSession> class data_source_var {
protected:
  std::shared_ptr<data_source> m_datasource;
};

template <> class data_source_var<multi_data_stream> {
protected:
  // TODO: need revise this
  std::vector<data_source> m_datasource;
};
template <> class data_source_var<noinput_data_stream> {};
} // namespace internal
} // namespace ypc
