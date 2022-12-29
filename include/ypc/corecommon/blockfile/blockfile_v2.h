#include "ypc/corecommon/blockfile/blockfile_base.h"

namespace ypc {
namespace blockfile {
struct header_v2 {
  uint64_t magic_number;
  uint64_t version_number;
  uint64_t block_number;
  uint64_t item_number;
  char data_hash[32];
};

} // namespace blockfile
template <typename File_t, uint64_t MagicNumber_t, uint64_t BlockNumLimit_t,
          uint64_t BlockSizeLimit_t>
using blockfile_v2 =
    internal::blockfile_impl<blockfile::header_v1, File_t, MagicNumber_t,
                             BLockNumLimit_t, BlockSizeLimit_t>;

} // namespace ypc
