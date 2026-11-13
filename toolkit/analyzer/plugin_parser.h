#pragma once

#include "iodef.h"
#include "parser_plugin.h"
#include "parser_registry.h"
#include "ypc/common/parser_type.h"
#include "ypc/core/sealed_file.h"
#include "ypc/keymgr/default/keymgr_sgx_module.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <toolkit/analyzer/fid_parser_plugin.h>

class plugin_parser {
public:
  plugin_parser(const input_param_t &param,
                const fid::parser_registry_selection &selection);
  ~plugin_parser();

  uint32_t parse();
  const std::string &get_result_str() const { return m_result_str; }

private:
  uint32_t feed_datasource();
  uint32_t feed_model();
  uint32_t dump_result(const ypc::bytes &result);
  uint32_t next_data_batch(const uint8_t *hash_and_pkey, uint32_t size,
                           uint8_t **data, uint32_t *len);

  static uint32_t next_data_batch_callback(void *context,
                                           const uint8_t *hash_and_pkey,
                                           uint32_t size, uint8_t **data,
                                           uint32_t *len);
  static uint32_t km_session_request_callback(void *context,
                                              sgx_dh_msg1_t *dh_msg1,
                                              uint32_t *session_id);
  static uint32_t km_exchange_report_callback(void *context,
                                              sgx_dh_msg2_t *dh_msg2,
                                              sgx_dh_msg3_t *dh_msg3,
                                              uint32_t session_id);
  static uint32_t km_send_request_callback(void *context, uint32_t session_id,
                                           secure_message_t *request,
                                           uint64_t request_size,
                                           uint64_t max_payload_size,
                                           secure_message_t *response,
                                           uint64_t response_size);
  static uint32_t km_end_session_callback(void *context, uint32_t session_id);
  static void log_callback(void *context, uint32_t rank, const char *message,
                           uint32_t message_size);

  input_param_t m_param;
  fid::parser_registry_selection m_selection;
  ypc::utc::parser_type_t m_parser_type{};
  std::shared_ptr<ypc::keymgr_sgx_module> m_keymgr;
  std::unique_ptr<fid::parser_plugin> m_plugin;
  fid_parser_host_callbacks m_callbacks;
  std::unordered_map<ypc::bytes, std::shared_ptr<ypc::simple_sealed_file>>
      m_data_sources;
  std::string m_result_str;
  std::unique_ptr<char[]> m_mem_buf;
  size_t m_mem_buf_size;
};
