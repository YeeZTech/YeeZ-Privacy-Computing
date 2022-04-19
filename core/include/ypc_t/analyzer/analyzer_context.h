#pragma once
#include "stbox/ebyte.h"

namespace ypc {
class analyzer_context {
public:
  virtual uint32_t
  request_private_key_for_public_key(const stbox::bytes &pubkey,
                                     stbox::bytes &private_key,
                                     stbox::bytes &dian_pkey) = 0;
  virtual const stbox::bytes &get_enclave_hash() const = 0;
};
} // namespace ypc
