
#include "ypc/sha.h"
#include <cryptopp/sha.h>

namespace ypc {

std::string SHA256(const char *data, size_t len) {
  CryptoPP::byte const *pbData = (CryptoPP::byte *)data;
  unsigned int nDataLen = len;
  CryptoPP::byte abDigest[CryptoPP::SHA256::DIGESTSIZE];
  CryptoPP::SHA256().CalculateDigest(abDigest, pbData, nDataLen);
  return std::string((char *)abDigest, CryptoPP::SHA256::DIGESTSIZE);
}
} // namespace ypc
