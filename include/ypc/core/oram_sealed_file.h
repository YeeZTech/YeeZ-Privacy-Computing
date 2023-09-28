#pragma once
#include "ypc/core/oramblockfile.h"
#include "ypc/common/limits.h"
#include "ypc/core/byte.h"


namespace ypc {
namespace internal {
  class oram_sealed_file_base {
public:
  // TODO:幻数
  using oramblockfile_t = ypc::oram::oramblockfile<1024 * 1024 * 256, 5 * 32, 4>;

  oram_sealed_file_base(const std::string &file_path);

  virtual ~oram_sealed_file_base() = default;

  virtual void reset() = 0;

  virtual bool download_oram_params(uint32_t *block_num, uint32_t *bucket_num_N, 
    uint8_t *level_num_L, uint32_t *bucket_str_size, uint32_t *row_length, uint32_t *batch_str_size) = 0;

  virtual uint32_t get_block_id(uint64_t c_id) = 0;

  virtual bool download_position_map(memref &posmap) = 0;

  virtual void update_position_map(uint8_t * position_map, uint32_t len) = 0;

  virtual bool download_path(uint32_t leaf, memref &en_path) = 0;

  virtual void upload_path(uint32_t leaf, uint8_t * encrpypted_path, uint32_t len) = 0;

  virtual bool download_stash(memref &st) = 0;

  virtual void update_stash(uint8_t * stash, uint32_t len) = 0;


public:
  oram_sealed_file_base(const oram_sealed_file_base &) = delete;
  oram_sealed_file_base(oram_sealed_file_base &&) = delete;
  oram_sealed_file_base &operator=(oram_sealed_file_base &&) = delete;
  oram_sealed_file_base &operator=(const oram_sealed_file_base &) = delete;

protected:
  oramblockfile_t m_file;
};
} // namespace internal

class simple_oram_sealed_file : public internal::oram_sealed_file_base {
public:
  simple_oram_sealed_file(const std::string &file_path);
  virtual void reset();
  virtual bool download_oram_params(uint32_t *block_num, uint32_t *bucket_num_N, 
    uint8_t *level_num_L, uint32_t *bucket_str_size, uint32_t *row_length, uint32_t *batch_str_size);
  virtual uint32_t get_block_id(uint64_t c_id);
  virtual bool download_position_map(memref &posmap);
  virtual void update_position_map(uint8_t * position_map, uint32_t len);
  virtual bool download_path(uint32_t leaf, memref &en_path);
  virtual void upload_path(uint32_t leaf, uint8_t * encrpypted_path, uint32_t len);
  virtual bool download_stash(memref &st);
  virtual void update_stash(uint8_t * stash, uint32_t len);
};



}