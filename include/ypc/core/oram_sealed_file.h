#pragma once
#include "ypc/common/limits.h"
#include "ypc/core/byte.h"
#include "ypc/core/oramblockfile.h"

namespace ypc {
namespace internal {
class oram_sealed_file_base {
public:
  // TODO:幻数
  using oramblockfile_t =
      ypc::oram::oramblockfile<1024 * 1024 * 256, 5 * 32, 4>;

  oram_sealed_file_base(const std::string &file_path);

  virtual ~oram_sealed_file_base() = default;

  virtual void reset() = 0;

  virtual bool download_oram_params(uint32_t *block_num, uint32_t *bucket_num_N,
                                    uint8_t *level_num_L,
                                    uint32_t *bucket_str_size,
                                    uint32_t *batch_str_size) = 0;

  virtual bool get_block_id(bytes &item_index_field_hash,
                            uint32_t *block_id) = 0;

  virtual bool download_position_map(memref &posmap) = 0;

  virtual bool update_position_map(uint8_t *position_map, uint32_t len) = 0;

  virtual bool download_path(uint32_t leaf, memref &en_path) = 0;

  virtual bool upload_path(uint32_t leaf, uint8_t *encrpypted_path,
                           uint32_t len) = 0;

  virtual bool download_stash(memref &st) = 0;

  virtual bool update_stash(uint8_t *stash, uint32_t len) = 0;

  virtual bool read_root_hash(memref &root_hash) = 0;

  virtual bool download_merkle_hash(uint32_t leaf, memref &merkle_hash) = 0;

  virtual bool update_merkle_hash(uint32_t leaf, uint8_t *merkle_hash,
                                  uint32_t len) = 0;

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
                                    uint8_t *level_num_L,
                                    uint32_t *bucket_str_size,
                                    uint32_t *batch_str_size);
  virtual bool get_block_id(bytes &item_index_field_hash, uint32_t *block_id);
  virtual bool download_position_map(memref &posmap);
  virtual bool update_position_map(uint8_t *position_map, uint32_t len);
  virtual bool download_path(uint32_t leaf, memref &en_path);
  virtual bool upload_path(uint32_t leaf, uint8_t *encrpypted_path,
                           uint32_t len);
  virtual bool download_stash(memref &st);
  virtual bool update_stash(uint8_t *stash, uint32_t len);
  // virtual bool read_root_hash(bytes &root_hash);
  virtual bool read_root_hash(memref &root_hash);
  virtual bool download_merkle_hash(uint32_t leaf, memref &merkle_hash);
  virtual bool update_merkle_hash(uint32_t leaf, uint8_t *merkle_hash,
                                  uint32_t len);
};
} // namespace ypc
