#pragma once
#include "common/crypto_prefix.h"
#include "sgx_trts.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "stbox/tsgx/crypto/ecc.h"
#include "stbox/tsgx/log.h"
#include "stbox/tsgx/ocall.h"
#include "ypc_t/analyzer/ntpackage_item_parser.h"
#include "ypc_t/analyzer/parser_wrapper_base.h"
#include "ypc_t/analyzer/sealed_raw_data.h"
#include "ypc_t/ecommon/package.h"
#include <string.h>

namespace ypc {

namespace internal {

template <typename UserItemT, typename ParserT>
class typed_parser_wrapper_for_offchain : public parser_wrapper_base {
public:
  typedef UserItemT (*item_parser_t)(const stbox::bytes::byte_t *, size_t);

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
    // m_encrypted_result_str is the first round encrypt

    auto skey_size = stbox::crypto::get_secp256k1_private_key_size();
    auto pkey_size = stbox::crypto::get_secp256k1_public_key_size();

    stbox::bytes skey(skey_size);

    stbox::crypto::gen_secp256k1_skey(skey_size, skey.data());
    stbox::bytes pkey(pkey_size);
    stbox::crypto::generate_secp256k1_pkey_from_skey(skey.data(), pkey.data(),
                                                     pkey_size);

    auto rs = m_encrypted_result_str;
    uint32_t cipher_size =
        stbox::crypto::get_encrypt_message_size_with_prefix(rs.size());
    m_encrypted_result_str = bytes(cipher_size);

    auto status = stbox::crypto::encrypt_message_with_prefix(
        pkey.data(), pkey_size, rs.data(), rs.size(),
        utc::crypto_prefix_arbitrary, (uint8_t *)&m_encrypted_result_str[0],
        cipher_size);
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << status;
      return status;
    }
    auto hash_m = stbox::eth::keccak256_hash(m_encrypted_result_str);

    stbox::bytes pkey_a(pkey_size);
    status = stbox::crypto::generate_secp256k1_pkey_from_skey(
        (const uint8_t *)&m_private_key[0], (uint8_t *)&pkey_a[0], pkey_size);

    cipher_size =
        stbox::crypto::get_encrypt_message_size_with_prefix(skey.size());
    m_encrypted_c = stbox::bytes(cipher_size);
    status = stbox::crypto::encrypt_message_with_prefix(
        pkey_a.data(), pkey_size, skey.data(), skey.size(),
        utc::crypto_prefix_arbitrary, m_encrypted_c.data(),
        m_encrypted_c.size());
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for encrypt_message: " << status;
      return status;
    }

    uint32_t sig_size = stbox::crypto::get_secp256k1_signature_size();
    stbox::bytes cost_gas_str(sizeof(m_cost_gas));
    memcpy((uint8_t *)&cost_gas_str[0], (uint8_t *)&m_cost_gas,
           sizeof(m_cost_gas));
    m_result_signature_str = stbox::bytes(sig_size);
    m_cost_signature_str = stbox::bytes(sig_size);

    auto cost_msg =
        m_encrypted_param + m_data_source->data_hash() + cost_gas_str;
    status = stbox::crypto::sign_message(
        (uint8_t *)m_private_key.data(), m_private_key.size(),
        (uint8_t *)&cost_msg[0], cost_msg.size(),
        (uint8_t *)&m_cost_signature_str[0], sig_size);
    if (status != stbox::stx_status::success) {
      LOG(ERROR) << "error for sign cost: " << status;
      return status;
    }

    auto msg = m_encrypted_c + hash_m + m_data_source->data_hash() +
               cost_gas_str + m_enclave_hash + m_encrypted_param;
    status = stbox::crypto::sign_message(
        (uint8_t *)m_private_key.data(), m_private_key.size(),
        (uint8_t *)&msg[0], msg.size(), (uint8_t *)&m_result_signature_str[0],
        sig_size);
    return static_cast<uint32_t>(status);
  }

  const bytes &data_hash() const { return m_data_source->data_hash(); }

  virtual bool
  user_def_block_result_merge(const std::vector<stbox::bytes> &block_results) {
    ParserT m;
    return m.merge_parse_result(block_results, m_param, m_result_str);
  }

  virtual uint32_t get_result_encrypt_key_size() {
    return m_encrypted_c.size();
  }
  virtual uint32_t get_result_encrypt_key(uint8_t *key, uint32_t key_size) {
    if (!key || key_size < m_encrypted_c.size()) {
      return stbox::stx_status::invalid_parameter_error;
    }
    memcpy(key, m_encrypted_c.data(), m_encrypted_c.size());
    return stbox::stx_status::success;
  }

  virtual utc::parser_type_t get_parser_type() {
    return utc::offchain_result_parser;
  }

protected:
  std::unique_ptr<ParserT> m_parser;
  item_parser_t m_item_parser_func;

  stbox::bytes m_encrypted_c;

  std::unique_ptr<sealed_data_provider<UserItemT>> m_data_source;
};

} // namespace internal

template <typename UserItemT, typename ParserT>
using parser_wrapper_for_offchain =
    internal::typed_parser_wrapper_for_offchain<UserItemT, ParserT>;

template <typename UserItemT, typename ParserT>
class plugin_parser_wrapper_for_offchain
    : public internal::typed_parser_wrapper_for_offchain<UserItemT, ParserT> {
public:
  virtual uint32_t parse_data_item(const uint8_t *sealed_data, uint32_t len) {
    if (!internal::typed_parser_wrapper_for_offchain<
            UserItemT, ParserT>::m_item_parser_func) {
      set_item_parser(::ypc::ntpackage_item_parser<stbox::bytes::byte_t,
                                                   UserItemT>::parser);
    }

    return internal::typed_parser_wrapper_for_offchain<
        UserItemT, ParserT>::parse_data_item(sealed_data, len);
  }
};
} // namespace ypc
