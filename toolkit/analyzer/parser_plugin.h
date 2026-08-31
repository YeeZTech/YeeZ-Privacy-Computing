#pragma once

#include "parser_registry.h"
#include <cstdint>
#include <string>
#include <vector>
#include <toolkit/analyzer/fid_parser_plugin.h>

namespace fid {

class parser_plugin {
public:
  parser_plugin(const parser_registry_selection &selection,
                const fid_parser_host_callbacks &callbacks);
  ~parser_plugin();

  parser_plugin(const parser_plugin &) = delete;
  parser_plugin &operator=(const parser_plugin &) = delete;

  uint32_t begin_parse_data_item();
  uint32_t parse_data_item(const uint8_t *data, uint32_t size);
  uint32_t end_parse_data_item();
  uint32_t init_data_source(const uint8_t *data, uint32_t size);
  uint32_t init_model(const uint8_t *data, uint32_t size);
  uint32_t get_parser_type(uint32_t &type);
  uint32_t get_enclave_hash(ypc::bytes &hash);
  uint32_t get_analyze_result(ypc::bytes &result);

private:
  typedef void (*simple_call)(fid_parser_plugin_instance *,
                              fid_parser_call_status *);

  uint32_t invoke(const char *name, simple_call fn);
  uint32_t invoke_data(const char *name,
                       void (*fn)(fid_parser_plugin_instance *, const uint8_t *,
                                  uint32_t, fid_parser_call_status *),
                       const uint8_t *data, uint32_t size);
  uint32_t invoke_buffer(const char *name,
                         void (*fn)(fid_parser_plugin_instance *, uint8_t *,
                                    uint32_t, uint32_t *,
                                    fid_parser_call_status *),
                         ypc::bytes &value);
  static uint32_t checked_business_status(const char *name,
                                          const fid_parser_call_status &status);

  const fid_parser_plugin_api *m_api;
  fid_parser_plugin_instance *m_instance;
  void *m_dso;
  int m_enclave_descriptor;
};

} // namespace fid
