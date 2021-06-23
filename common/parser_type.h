#pragma once
#include <cstdint>

namespace ypc {
namespace utc {
typedef uint32_t parser_type_t;

constexpr static uint32_t parser_type_length = sizeof(parser_type_t);

constexpr static parser_type_t onchain_result_parser = 1;
constexpr static parser_type_t offchain_result_parser = 2;
} // namespace utc
} // namespace ypc
