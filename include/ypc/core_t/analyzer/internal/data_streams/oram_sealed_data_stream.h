#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/stbox/ebyte.h"

namespace ypc {
namespace internal {

class oram_sealed_data_stream {};

} // namespace internal
using oram_sealed_data_stream = internal::oram_sealed_data_stream;

template <> struct datasource_type_traits<oram_sealed_data_stream> {
  constexpr static uint32_t value = ypc::utc::oram_sealed_datasource_parser;
};
} // namespace ypc