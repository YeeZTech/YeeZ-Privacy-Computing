#pragma once
#include "common/parser_type.h"
namespace ypc {
template <typename T> struct result_type_traits {
  constexpr static uint32_t value = ypc::utc::unknown_result_parser;
};

template <typename T> struct datasource_type_traits {
  constexpr static uint32_t value = ypc::utc::unknown_datasource_parser;
};

template <typename T> struct model_type_traits {
  constexpr static uint32_t value = ypc::utc::has_model_parser;
};

template <> struct model_type_traits<void> {
  constexpr static uint32_t value = ypc::utc::no_model_parser;
};

} // namespace ypc
