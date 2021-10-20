#pragma once
#include <ff/net/common/archive.h>
#include <ff/net/middleware/ntpackage.h>

namespace ypc {
namespace datahub {
template <typename BytesType> struct data_host {
public:
  define_nt(pkey, BytesType);
  define_nt(sealed_skey, BytesType);
  define_nt(data_hash, BytesType);
  define_nt(signature, BytesType);
  typedef ::ff::net::ntpackage<0x132f4635, pkey, sealed_skey, data_hash,
                               signature>
      credential_package_t;

  define_nt(encrypted_skey, BytesType);
  typedef ::ff::net::ntpackage<0xbbeadc47, encrypted_skey, signature>
      usage_license_package_t;
};

} // namespace datahub
} // namespace ypc
