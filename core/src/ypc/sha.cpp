#include "ypc/sha.h"
#include <cryptopp/sha.h>

namespace ypc {

bytes SHA256(const char *data, size_t len) {
  CryptoPP::byte const *pbData = (CryptoPP::byte *)data;
  unsigned int nDataLen = len;
  bytes ret(CryptoPP::SHA256::DIGESTSIZE);
  CryptoPP::SHA256().CalculateDigest((CryptoPP::byte *)ret.data(), pbData,
                                     nDataLen);
  return ret;
}
} // namespace ypc
