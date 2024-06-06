#pragma once
#include "ypc/core/oram_sealed_file.h"
#include "iodef.h"
#include "ypc/common/parser_type.h"
#include "ypc/core/sgx/parser_sgx_module.h"
#include "ypc/keymgr/default/keymgr_sgx_module.h"


class oram_parser {
public:
  oram_parser(const input_param_t &param);

  virtual ~oram_parser();

  virtual uint32_t oram_parse();

  virtual uint32_t download_oram_params_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
    uint32_t *block_num, uint32_t *bucket_num_N, 
    uint8_t *level_num_L, uint32_t *bucket_str_size, uint32_t *batch_str_size);
  
  virtual uint32_t get_block_id_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                      uint32_t *block_id,
                                      const uint8_t *param_hash, uint32_t param_hash_size);

  virtual uint32_t download_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                               uint8_t ** position_map, uint32_t *len);

  virtual uint32_t update_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                             const uint8_t * position_map, uint32_t len);

  virtual uint32_t download_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                       uint32_t leaf, uint8_t ** encrpypted_path, uint32_t *len);

  virtual uint32_t download_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                        uint8_t ** stash, uint32_t *len);
  virtual uint32_t update_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                      const uint8_t * stash, uint32_t len);

  virtual uint32_t upload_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                     uint32_t leaf, const uint8_t * encrpypted_path, uint32_t len);

  virtual uint32_t download_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                              uint32_t leaf, uint8_t ** merkle_hash, uint32_t *len);

  virtual uint32_t update_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                            uint32_t leaf, const uint8_t * merkle_hash, uint32_t len);
  

  inline std::shared_ptr<keymgr_sgx_module> keymgr() const { return m_keymgr; }

  inline const std::string &get_result_str() const { return m_result_str; }

protected:
  uint32_t feed_datasource();
  uint32_t feed_model();
  uint32_t feed_param();
  uint32_t dump_result(const ypc::bytes &res);

protected:
  input_param_t m_param;
  ypc::utc::parser_type_t m_ptype{};

  std::shared_ptr<ypc::parser_sgx_module> m_parser;
  std::shared_ptr<keymgr_sgx_module> m_keymgr;
  std::unordered_map<ypc::bytes, std::shared_ptr<ypc::simple_oram_sealed_file>>
      m_data_sources;
  std::string m_result_str;
};