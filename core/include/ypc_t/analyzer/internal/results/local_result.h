#pragma once
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/helper/parser_type_traits.h"
#include "ypc_t/analyzer/internal/is_param_encrypted.h"
#include "ypc_t/analyzer/var/result_var.h"

namespace ypc {
namespace internal {
class local_result : virtual public result_var {
public:
  inline uint32_t generate_result() { return 0; }
  inline uint32_t get_analyze_result_size() {
    return result_var::m_result.size();
  }
  inline uint32_t get_analyze_result(uint8_t *result, uint32_t size) {
    if (size != result_var::m_result.size()) {
      return stbox::stx_status::out_buffer_length_error;
    }
    memcpy(result, result_var::m_result.data(), size);
    return stbox::stx_status::success;
  }
};
template <> struct is_param_encrypted<local_result> {
  static constexpr bool value = false;
};
} // namespace internal
typedef internal::local_result local_result;
template <> struct result_type_traits<internal::local_result> {
  constexpr static uint32_t value = ypc::utc::local_result_parser;
};
} // namespace ypc
