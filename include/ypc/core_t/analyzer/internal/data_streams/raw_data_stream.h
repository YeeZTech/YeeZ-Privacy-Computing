#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/stbox/ebyte.h"

namespace ypc {
namespace internal {

class raw_data_stream {};

} // namespace internal
using raw_data_stream = internal::raw_data_stream;
template <> struct datasource_type_traits<raw_data_stream> {
  constexpr static uint32_t value = ypc::utc::raw_datasource_parser;
};
} // namespace ypc
