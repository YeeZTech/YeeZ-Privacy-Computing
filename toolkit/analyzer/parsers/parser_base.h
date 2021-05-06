#pragma once
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

  virtual uint32_t parse();

  virtual bool
  merge(std::vector<std::shared_ptr<result_target>> &block_results);

  virtual uint32_t next_sealed_item_data(uint8_t **data, uint32_t *len) = 0;
  virtual void free_sealed_item_data(uint8_t *data) = 0;

  inline std::shared_ptr<ypc::datahub_sgx_module> sealer() const {
    return m_sealer;
  }
  inline std::shared_ptr<parser_sgx_module> parser() const { return m_parser; }
  inline std::shared_ptr<keymgr_sgx_module> keymgr() const { return m_keymgr; }

protected:
  virtual uint32_t do_parse() = 0;

protected:
  param_source *m_psource;
  result_target *m_rtarget;

  std::string m_sealer_enclave_path;
  std::string m_parser_enclave_path;
  std::string m_keymgr_enclave_path;

  std::shared_ptr<ypc::datahub_sgx_module> m_sealer;
  std::shared_ptr<parser_sgx_module> m_parser;
  std::shared_ptr<keymgr_sgx_module> m_keymgr;
};
