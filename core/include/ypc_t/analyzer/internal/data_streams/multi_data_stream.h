#pragma once
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/helper/parser_type_traits.h"
#include "ypc_t/analyzer/internal/is_multi_datasource.h"

namespace ypc {
namespace internal {

class multi_data_stream {};

template <> struct is_multi_datasource<multi_data_stream> {
  static constexpr bool value = true;
};

} // namespace internal
using multi_data_stream = internal::multi_data_stream;

template <> struct datasource_type_traits<multi_data_stream> {
  constexpr static uint32_t value = ypc::utc::multi_sealed_datasource_parser;
};

} // namespace ypc
