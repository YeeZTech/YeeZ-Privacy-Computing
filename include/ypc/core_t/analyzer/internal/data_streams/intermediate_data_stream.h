#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/stbox/ebyte.h"

namespace ypc {
namespace internal {

class intermediate_data_stream {};

template <> struct is_multi_datasource<intermediate_data_stream> {
  static constexpr bool value = true;
};

} // namespace internal
using intermediate_data_stream = internal::intermediate_data_stream;

template <> struct datasource_type_traits<intermediate_data_stream> {
  constexpr static uint32_t value = ypc::utc::multi_sealed_datasource_parser;
};
} // namespace ypc
