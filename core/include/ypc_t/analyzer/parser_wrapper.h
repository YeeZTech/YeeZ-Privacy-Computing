#pragma once
#include "sgx_trts.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/log.h"
#include "stbox/tsgx/ocall.h"
#include "ypc_t/analyzer/parser_wrapper_base.h"
#include "ypc_t/analyzer/sealed_raw_data.h"
#include "ypc_t/ecommon/package.h"
#include <string.h>

namespace ypc {

template <typename UserItemT, typename ParserT>
class parser_wrapper : public parser_wrapper_base {
public:
  typedef UserItemT (*item_parser_t)(const char *, size_t);

  virtual uint32_t begin_parse_data_item() {
    uint32_t r1 = parser_wrapper_base::begin_parse_data_item();
    m_data_source.reset(
        new sealed_data_provider<UserItemT>(m_datahub_session.get()));
    if (m_item_parser_func) {
      m_data_source->set_item_parser(m_item_parser_func);
    }
    m_parser.reset(new ParserT(m_data_source.get()));
    return r1;
  }

  inline void set_item_parser(item_parser_t func) {
    m_item_parser_func = func;
    if (m_data_source) {
      m_data_source->set_item_parser(m_item_parser_func);
    }
  }

  virtual uint32_t parse_data_item(const uint8_t *sealed_data, uint32_t len) {
    uint32_t r1 = parser_wrapper_base::parse_data_item(sealed_data, len);
    if (r1 == static_cast<uint32_t>(stbox::stx_status::success)) {
      m_result_str = m_parser->do_parse(m_param);
      // TODO Calculate actual gas cost
      m_cost_gas = 0;
    }
    return r1;
  }

  virtual uint32_t end_parse_data_item() {
    uint32_t r1 = parser_wrapper_base::end_parse_data_item();
    if (r1 != static_cast<uint32_t>(stbox::stx_status::success)) {
      return r1;
    }
    uint32_t sig_size = stbox::crypto::get_secp256k1_signature_size();
    std::string cost_gas_str(sizeof(m_cost_gas), '0');
    memcpy((uint8_t *)&cost_gas_str[0], (uint8_t *)&m_cost_gas,
           sizeof(m_cost_gas));
    m_result_signature_str = std::string(sig_size, '0');
    auto msg = m_encrypted_param +
               stbox::byte_to_string(m_data_source->data_hash()) +
               cost_gas_str + m_encrypted_result_str;
    auto status = stbox::crypto::sign_message(
        (uint8_t *)m_private_key.c_str(), m_private_key.size(),
        (uint8_t *)&msg[0], msg.size(), (uint8_t *)&m_result_signature_str[0],
        sig_size);
    return static_cast<uint32_t>(status);
  }

  const bytes &data_hash() const { return m_data_source->data_hash(); }

  virtual bool
  user_def_block_result_merge(const std::vector<std::string> &block_results) {
    ParserT m;
    return m.merge_parse_result(block_results, m_param, m_result_str);
  }

protected:
  std::unique_ptr<ParserT> m_parser;
  item_parser_t m_item_parser_func;

  std::unique_ptr<sealed_data_provider<UserItemT>> m_data_source;
};

} // namespace ypc
