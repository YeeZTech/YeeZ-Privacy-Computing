#pragma once
#include "ypc/corecommon/blockfile/blockfile_base.h"
#include "ypc/corecommon/blockfile/blockfile_base_r.h"

namespace ypc {
namespace internal {
struct blockfile_header_v1 {
  uint64_t magic_number;
  uint64_t version_number;
  uint64_t block_number;
  uint64_t item_number;
};

} // namespace internal
template <typename File_t, uint64_t MagicNumber_t, uint64_t VersionNumber_t,
          uint64_t BlockNumLimit_t, uint64_t ItemNumPerBlockLimit_t>
using blockfile_v1 =
    internal::blockfile_impl<internal::blockfile_header_v1, File_t,
                             MagicNumber_t, VersionNumber_t, BlockNumLimit_t,
                             ItemNumPerBlockLimit_t>;

template <typename File_t, uint64_t MagicNumber_t, uint64_t VersionNumber_t,
          uint64_t BlockNumLimit_t, uint64_t ItemNumPerBlockLimit_t>
using blockfile_v1_r =
    internal::blockfile_impl_r<internal::blockfile_header_v1, File_t,
                               MagicNumber_t, VersionNumber_t, BlockNumLimit_t,
                               ItemNumPerBlockLimit_t>;

} // namespace ypc
