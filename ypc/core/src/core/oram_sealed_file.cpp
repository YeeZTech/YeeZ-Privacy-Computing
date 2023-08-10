#include "ypc/core/oram_sealed_file.h"

namespace ypc {
namespace internal {
oram_sealed_file_base::oram_sealed_file_base(const std::string &file_path) : m_file(file_path) {}

} // namespace internal

simple_oram_sealed_file::simple_oram_sealed_file(const std::string &file_path) 
    : oram_sealed_file_base(file_path) {}

void simple_oram_sealed_file::reset() { m_file.reset(); }

bool simple_oram_sealed_file::download_oram_params(uint32_t *block_num, uint32_t *bucket_num_N, 
    uint8_t *level_num_L, uint32_t *bucket_str_size, uint32_t *batch_str_size) {
  
  *block_num = m_file.get_block_num();
  *bucket_num_N = m_file.get_bucket_num();
  *level_num_L = m_file.get_level_num();
  *bucket_str_size = m_file.get_bucket_str_size();
  *batch_str_size = m_file.get_batch_str_size();

  return true;
}

bool simple_oram_sealed_file::get_block_id(bytes &item_index_field_hash, uint32_t *block_id) { 
  return m_file.get_block_id(item_index_field_hash, block_id); 
}

bool simple_oram_sealed_file::download_position_map(memref &posmap) {return m_file.download_position_map(posmap); }

bool simple_oram_sealed_file::update_position_map(uint8_t * position_map, uint32_t len) {
  return m_file.update_position_map(position_map, len);
}

bool simple_oram_sealed_file::download_path(uint32_t leaf, memref &en_path) {
  return m_file.download_path(leaf, en_path);
}

bool simple_oram_sealed_file::upload_path(uint32_t leaf, uint8_t * encrpypted_path, uint32_t len) {
  return m_file.upload_path(leaf, encrpypted_path, len);
}

bool simple_oram_sealed_file::download_stash(memref &st) { return m_file.download_stash(st); }

bool simple_oram_sealed_file::update_stash(uint8_t * stash, uint32_t len) { 
  return m_file.update_stash(stash, len); 
}

bool simple_oram_sealed_file::read_root_hash(memref &root_hash) {
  return m_file.read_root_hash(root_hash);
}

bool simple_oram_sealed_file::download_merkle_hash(uint32_t leaf, memref &merkle_hash) {
  return m_file.download_merkle_hash(leaf, merkle_hash);
}

bool simple_oram_sealed_file::update_merkle_hash(uint32_t leaf, uint8_t * merkle_hash, uint32_t len) {
  return m_file.update_merkle_hash(leaf, merkle_hash, len);
}

}