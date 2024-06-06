#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/stbox/ebyte.h"

namespace ypc {
namespace internal {

class convert_sealed_data_stream {};

} // namespace internal
using convert_sealed_data_stream = internal::convert_sealed_data_stream;

template <> struct datasource_type_traits<convert_sealed_data_stream> {
  constexpr static uint32_t value = ypc::utc::single_sealed_datasource_parser;
};
} // namespace ypc