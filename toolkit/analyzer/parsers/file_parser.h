#pragma once

#include "parser_base.h"
#include "ypc/sealed_file.h"

class file_parser : public parser_base {
public:
  file_parser(param_source *psource, result_target *rtarget,
              const std::string &sealer_enclave_path,
              const std::string &parser_enclave_path,
              const std::string &keymgr_enclave_path,
              const std::string &sealfile_path);

protected:
  virtual uint32_t do_parse();

  virtual uint32_t next_sealed_item_data(uint8_t **data, uint32_t *len);
  virtual void free_sealed_item_data(uint8_t *data);

protected:
  std::string m_sealfile_path;
  std::shared_ptr<ypc::internal::sealed_file_base> m_sf;
};
