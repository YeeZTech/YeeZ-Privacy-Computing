#pragma once
#include "ypc/core_t/analyzer/helper/parser_type_traits.h"
#include "ypc/core_t/analyzer/var/data_hash_var.h"
#include "ypc/core_t/analyzer/var/enclave_hash_var.h"
#include "ypc/core_t/analyzer/var/encrypted_param_var.h"
#include "ypc/core_t/analyzer/var/request_key_var.h"
#include "ypc/core_t/analyzer/var/result_var.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {
template <typename Crypto>
class taskgraph_result : virtual public request_key_var<true>,
                         virtual public enclave_hash_var,
                         virtual public result_var,
                         virtual public encrypted_param_var,
                         virtual public data_hash_var {
  typedef Crypto ecc;
  typedef request_key_var<true> request_key_var_t;

public:
  uint32_t generate_result() {}

protected:
  stbox::bytes m_target_dian_pkey;
} // namespace internal
template <typename Crypto>
using taskgraph_result = internal::taskgraph_result<Crypto>;

template <typename Crypto> struct result_type_traits<taskgraph_result<Crypto>> {
  constexpr static uint32_t value = ypc::utc::taskgraph_result_parser;
};

} // namespace ypc
