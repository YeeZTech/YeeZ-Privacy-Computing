#pragma once
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/helper/parser_type_traits.h"

namespace ypc {
namespace internal {

class sealed_data_stream {};

} // namespace internal
using sealed_data_stream = internal::sealed_data_stream;

template <> struct datasource_type_traits<sealed_data_stream> {
  constexpr static uint32_t value = ypc::utc::single_sealed_datasource_parser;
};
} // namespace ypc
