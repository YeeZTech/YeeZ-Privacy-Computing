#pragma once
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/helper/parser_type_traits.h"

namespace ypc {
namespace internal {

class noinput_data_stream {};

} // namespace internal
using noinput_data_stream = internal::noinput_data_stream;

template <> struct datasource_type_traits<noinput_data_stream> {
  constexpr static uint32_t value = ypc::utc::noinput_datasource_parser;
};
} // namespace ypc
