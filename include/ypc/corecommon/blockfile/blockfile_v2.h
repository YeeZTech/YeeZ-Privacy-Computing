#pragma once
#include "ypc/corecommon/blockfile/blockfile_base.h"
#include "ypc/corecommon/blockfile/blockfile_base_r.h"

namespace ypc {
namespace internal {
struct header_v2 {
  uint64_t magic_number;
  uint64_t version_number;
  uint64_t block_number;
  uint64_t item_number;
  char data_hash[32];
};

} // namespace internal
template <typename File_t, uint64_t MagicNumber_t, uint64_t VersionNumber_t,
          uint64_t BlockNumLimit_t, uint64_t ItemNumPerBlockLimit_t>
using blockfile_v2 =
    internal::blockfile_impl<internal::header_v2, File_t, MagicNumber_t,
                             VersionNumber_t, BlockNumLimit_t,
                             ItemNumPerBlockLimit_t>;

template <typename File_t, uint64_t MagicNumber_t, uint64_t VersionNumber_t,
          uint64_t BlockNumLimit_t, uint64_t ItemNumPerBlockLimit_t>
using blockfile_v2_r =
    internal::blockfile_impl_r<internal::header_v2, File_t, MagicNumber_t,
                               VersionNumber_t, BlockNumLimit_t,
                               ItemNumPerBlockLimit_t>;

} // namespace ypc
