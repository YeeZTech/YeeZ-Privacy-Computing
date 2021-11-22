#include "ypc/terminus/crypto_pack.h"

namespace ypc {
namespace terminus {
std::unique_ptr<crypto_pack> sm_compatible() {
  return std::unique_ptr<crypto_pack>();
}
} // namespace terminus
} // namespace ypc
