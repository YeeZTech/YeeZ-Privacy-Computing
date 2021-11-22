#pragma once
#include "ypc/terminus/crypto_pack.h"
namespace ypc {
namespace terminus {
class interaction_base {
public:
  inline interaction_base(crypto_pack *crypto) : m_crypto(crypto) {}

protected:
  crypto_pack *m_crypto;
};
} // namespace terminus
} // namespace ypc
