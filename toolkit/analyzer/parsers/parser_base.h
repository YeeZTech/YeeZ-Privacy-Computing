#pragma once
#include "../extra_data_source.h"
#include "keymgr/default/keymgr_sgx_module.h"
#include "params/param_source.h"
#include "result/result_target.h"
#include "ypc/sgx/datahub_sgx_module.h"
#include "ypc/sgx/parser_sgx_module.h"
#include <memory>

class parser_base {
public:
  parser_base(param_source *psource, result_target *rtarget,
              const std::string &sealer_enclave_path,
              const std::string &parser_enclave_path,
              const std::string &keymgr_enclave_path);

  virtual ~parser_base();

  inline void set_extra_data_source(const ypc::extra_data_source_t &eds) {
    m_extra_data_source = eds;
  };

  virtual uint32_t parse(const ypc::bytes &expect_data_hash);

  virtual bool
  merge(std::vector<std::shared_ptr<result_target>> &block_results);

  virtual uint32_t next_sealed_item_data(uint8_t **data, uint32_t *len) = 0;
  virtual void free_sealed_item_data(uint8_t *data) = 0;

  inline std::shared_ptr<ypc::datahub_sgx_module> sealer() const {
    return m_sealer;
  }
  inline std::shared_ptr<ypc::parser_sgx_module> parser() const {
    return m_parser;
  }
  inline std::shared_ptr<keymgr_sgx_module> keymgr() const { return m_keymgr; }

protected:
  virtual uint32_t do_parse() = 0;

  uint32_t forward_extra_data_usage_license(const ypc::bytes &enclave_pkey);

protected:
  param_source *m_psource;
  result_target *m_rtarget;

  std::string m_sealer_enclave_path;
  std::string m_parser_enclave_path;
  std::string m_keymgr_enclave_path;

  std::shared_ptr<ypc::datahub_sgx_module> m_sealer;
  std::shared_ptr<ypc::parser_sgx_module> m_parser;
  std::shared_ptr<keymgr_sgx_module> m_keymgr;

  ypc::extra_data_source_t m_extra_data_source;
};
