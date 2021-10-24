#pragma once
#include <corecommon/nt_cols.h>
#include <corecommon/package.h>
#include <ff/net/common/archive.h>
#include <ff/net/middleware/ntpackage.h>

namespace ypc {
namespace datahub {
template <typename BytesType> struct data_host {
public:
  using pkey = typename nt<BytesType>::pkey;
  using sealed_skey = typename nt<BytesType>::sealed_skey;
  using data_hash = typename nt<BytesType>::data_hash;
  using signature = typename nt<BytesType>::signature;
  using encrypted_skey = typename nt<BytesType>::encrypted_skey;

  typedef ::ff::net::ntpackage<0x132f4635, pkey, sealed_skey, data_hash,
                               signature>
      credential_package_t;

  typedef ::ff::net::ntpackage<0xbbeadc47, encrypted_skey, signature>
      usage_license_package_t;
};

} // namespace datahub
} // namespace ypc
