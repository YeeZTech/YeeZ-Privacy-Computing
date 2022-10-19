#pragma once
#include <cstdint>

namespace ypc {
namespace utc {

constexpr static uint32_t parser_type_length = sizeof(uint32_t);

constexpr static uint32_t unknown_result_parser = 0;
constexpr static uint32_t onchain_result_parser = 1;
constexpr static uint32_t offchain_result_parser = 2;
constexpr static uint32_t local_result_parser = 0x3;
constexpr static uint32_t forward_result_parser = 0x4;

constexpr static uint32_t unknown_datasource_parser = 0;
constexpr static uint32_t single_sealed_datasource_parser = 1;
constexpr static uint32_t multi_sealed_datasource_parser = 2;
constexpr static uint32_t noinput_datasource_parser = 3;
constexpr static uint32_t raw_datasource_parser = 4;

constexpr static uint32_t no_model_parser = 0;
constexpr static uint32_t has_model_parser = 1;

union parser_type_t {
  uint32_t value;
  struct _data {
    uint32_t result_type : 4;
    uint32_t data_source_type : 4;
    uint32_t has_model : 1;
  } d;
};
} // namespace utc
} // namespace ypc
