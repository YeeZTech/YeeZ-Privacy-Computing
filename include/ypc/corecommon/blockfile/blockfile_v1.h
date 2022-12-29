#include "ypc/corecommon/blockfile/blockfile_base.h"

namespace ypc {
namespace internal {
struct blockfile_header_v1 {
  uint64_t magic_number;
  uint64_t version_number;
  uint64_t block_number;
  uint64_t item_number;
};

} // namespace internal
template <typename File_t, uint64_t MagicNumber_t, uint64_t BlockNumLimit_t,
          uint64_t BlockSizeLimit_t>
using blockfile_v1 =
    internal::blockfile_impl<internal::blockfile_header_v1, File_t,
                             MagicNumber_t, BlockNumLimit_t, BlockSizeLimit_t>;

} // namespace ypc
