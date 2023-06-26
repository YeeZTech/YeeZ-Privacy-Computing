#pragma once
#include "ypc/corecommon/package.h"
// #include "ypc/core/byte.h"
#include "ypc/stbox/ebyte.h"
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>
#include "ypc/corecommon/nt_cols.h"
#include "ypc/common/limits.h"


namespace ypc {
namespace oram {

constexpr static uint32_t max_batch_size = ::ypc::utc::max_item_size;
// constexpr static uint32_t max_batch_size = 5 * 32;


struct header {
    // 真实数据块大小(字节为单位)
    uint32_t block_num;
    uint32_t bucket_num_N;
    uint8_t level_num_L;
    uint32_t bucket_str_size;
    uint32_t row_length;
    uint32_t batch_str_size;
    long int id_map_filepos;
    long int position_map_filepos;
    long int oram_tree_filepos;
};

define_nt(content_id, uint64_t);
define_nt(block_id, uint32_t);
using id_map_pair = ::ff::util::ntobject<content_id, block_id>;
define_nt(id_map, std::vector<id_map_pair>);
using id_map_t = ::ff::net::ntpackage<0x82c4e8da, id_map>;


define_nt(position_map, std::vector<uint32_t>);
using position_map_t = ::ff::net::ntpackage<0x82c4e8dd, position_map>;


define_nt(content, stbox::bytes);
using batch_row_t = ::ff::util::ntobject<content_id, content>;
define_nt(batch, std::vector<batch_row_t>);
using batch_pkg_t = ::ff::net::ntpackage<0x82c4e8de, batch>;


define_nt(leaf_label, uint32_t);
define_nt(encrypted_batch, stbox::bytes);
using block_t = ::ff::util::ntobject<block_id, leaf_label, encrypted_batch>;
define_nt(bucket, std::vector<block_t>);
using bucket_pkg_t = ::ff::net::ntpackage<0x82c4e8df, bucket>;

// using bucket_t = ::ff::util::ntobject<bucket>;
define_nt(path, std::vector<stbox::bytes>);
using path_pkg_t = ::ff::net::ntpackage<0x82c4e8ea, path>;


}
}