#pragma once

#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>


namespace ypc {
namespace oram {

constexpr static uint8_t BucketSizeZ = 4;
constexpr static uint32_t stash_size = 90;

template <typename BytesType> struct nt {
  define_nt(item_index_field_hash, BytesType);
  define_nt(block_id, uint32_t);
  using id_map_pair = ::ff::util::ntobject<item_index_field_hash, block_id>;
  define_nt(id_map, std::vector<id_map_pair>);
  using id_map_t = ::ff::net::ntpackage<0x82c4e8da, id_map>;

  define_nt(position_map, std::vector<uint32_t>);
  using position_map_t = ::ff::net::ntpackage<0x82c4e8dd, position_map>;

  define_nt(leaf_label, uint32_t);
  define_nt(valid_item_num, uint32_t);
  define_nt(encrypted_batch, BytesType);
  using block_t = ::ff::util::ntobject<block_id, leaf_label, valid_item_num, encrypted_batch>;

  define_nt(bucket, std::vector<block_t>);
  using bucket_pkg_t = ::ff::net::ntpackage<0x82c4e8df, bucket>;

  define_nt(path, std::vector<BytesType>);
  using path_pkg_t = ::ff::net::ntpackage<0x82c4e8ea, path>;

  define_nt(merkle_hash, std::vector<BytesType>);
  using merkle_hash_pkg_t = ::ff::net::ntpackage<0x82c4e7ea, merkle_hash>;
};


struct header {
  uint32_t block_num;
  uint32_t bucket_num_N;
  uint8_t level_num_L;
  uint32_t bucket_str_size;
  uint32_t batch_str_size;
  long int id_map_filepos;
  long int oram_tree_filepos;
  long int position_map_filepos;
  long int merkle_tree_filepos;
  long int stash_filepos;
  uint64_t stash_size;
};


}
}