#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>

namespace ypc {
template <typename BytesType> struct nt {
  define_nt(reserve, uint32_t);
  define_nt(data, BytesType);
  define_nt(id, uint32_t);
  define_nt(user_id, std::string);
  define_nt(timestamp, uint64_t);

  define_nt(encrypted_skey, BytesType);
  define_nt(pkey, BytesType);
  define_nt(sealed_skey, BytesType);
  define_nt(data_hash, BytesType);
  define_nt(signature, BytesType);

  define_nt(extra_data_group_name, std::string);
  define_nt(extra_data_hashes, std::vector<BytesType>);
  define_nt(msg, std::string);
  define_nt(succ, bool);

  typedef ::ff::net::ntpackage<0x35d22084, extra_data_group_name,
                               extra_data_hashes>
      extra_data_group_t;
  define_nt(extra_data_items, std::vector<extra_data_group_t>);
  typedef ::ff::net::ntpackage<0x82c4e8d6, extra_data_items>
      extra_data_package_t;

  define_nt(encrypted_param, BytesType);
  define_nt(pkey4v, BytesType);
  define_nt(encrypted_result, BytesType);
  define_nt(result_signature, BytesType);
  define_nt(cost_signature, BytesType);
  define_nt(result_encrypt_key, BytesType);
  typedef ::ff::net::ntpackage<0xf13e1f40, encrypted_result, data_hash,
                               result_signature, cost_signature,
                               result_encrypt_key>
      ypc_result_package_t;
  typedef ::ff::net::ntpackage<0x314234b9, pkey, sealed_skey, user_id,
                               timestamp>
      keymgr_key_package_t;

  define_nt(access_list_type, uint8_t);
  define_nt(access_data_type, uint8_t);
  typedef ::ff::net::ntpackage<0x71185aff, data, access_data_type>
      access_item_t;
  define_nt(access_list, std::vector<access_item_t>);
  typedef ::ff::net::ntpackage<0x3c92350f, access_list_type, access_list>
      access_list_package_t;

  typedef ::ff::net::ntpackage<0, succ, msg, data> auth_parser_response_pkg_t;
};
} // namespace ypc
