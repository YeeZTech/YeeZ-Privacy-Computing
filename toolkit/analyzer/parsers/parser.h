#pragma once
#include "common/parser_type.h"
#include "iodef.h"
#include "keymgr/default/keymgr_sgx_module.h"
#include "ypc/sealed_file.h"
#include "ypc/sgx/parser_sgx_module.h"
#include <memory>
#include <unordered_map>

class parser {
public:
  parser(const input_param_t &param);

  virtual ~parser();

  virtual uint32_t parse();

  virtual uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                                   uint8_t **data, uint32_t *len);
  virtual void free_data_batch(uint8_t *data);

  inline std::shared_ptr<keymgr_sgx_module> keymgr() const { return m_keymgr; }

  inline const std::string &get_result_str() const { return m_result_str; }

protected:
  uint32_t feed_datasource();
  uint32_t feed_model();
  uint32_t feed_param();
  uint32_t dump_result(const ypc::bytes &res);

protected:
  input_param_t m_param;
  ypc::utc::parser_type_t m_ptype;

  std::shared_ptr<ypc::parser_sgx_module> m_parser;
  std::shared_ptr<keymgr_sgx_module> m_keymgr;
  std::unordered_map<ypc::bytes, std::shared_ptr<ypc::simple_sealed_file>>
      m_data_sources;
  std::string m_result_str;
};
