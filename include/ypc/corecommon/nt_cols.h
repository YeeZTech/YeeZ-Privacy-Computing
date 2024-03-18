#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>

namespace ypc {
template <typename BytesType> struct nt {
  define_nt(reserve, uint32_t);
  define_nt(data, BytesType);
  define_nt(id, uint32_t);
  define_nt(tag, uint32_t);
  define_nt(user_id, std::string);
  define_nt(timestamp, uint64_t);

  define_nt(enclave_hash, BytesType);
  define_nt(encrypted_shu_skey, BytesType);
  define_nt(encrypted_skey, BytesType);
  define_nt(pkey, BytesType, "public-key");
  define_nt(private_key, BytesType, "private-key");
  define_nt(sealed_skey, BytesType);
  define_nt(data_hash, BytesType);
  define_nt(signature, BytesType);
  define_nt(shu_forward_signature, BytesType);
  define_nt(model_data, BytesType);
  define_nt(param_data, BytesType);
  define_nt(encrypted_sig, BytesType);
  //define_nt(crypto, std::string);

  define_nt(msg, std::string);
  define_nt(succ, bool);
  define_nt(batch_data, std::vector<BytesType>);

  using allowance_t = ::ff::util::ntobject<signature, pkey, data_hash>;
  define_nt(allowance, allowance_t);
  define_nt(allowances, std::vector<allowance_t>);

  using forward_target_info_t =
      ff::util::ntobject<enclave_hash, pkey, encrypted_sig>;
  define_nt(forward, forward_target_info_t);
  using param_t = ::ff::util::ntobject<param_data, pkey, allowances, forward>;
  define_nt(param, param_t);
  using model_t = ::ff::util::ntobject<model_data, pkey>;
  define_nt(model, model_t);

  using sealed_data_info_t = ::ff::util::ntobject<data_hash, pkey, tag>;
  define_nt(sealed_data_info_vector, std::vector<sealed_data_info_t>);
  using multi_sealed_data_info_t =
      ::ff::util::ntobject<sealed_data_info_vector>;

  using batch_data_pkg_t = ::ff::net::ntpackage<0x82c4e8d8, batch_data>;

  define_nt(encrypted_param, BytesType);
  define_nt(pkey4v, BytesType);
  define_nt(encrypted_result, BytesType);
  define_nt(result_signature, BytesType);
  define_nt(cost_signature, BytesType);
  define_nt(result_encrypt_key, BytesType);
  using onchain_result_package_t =
      ::ff::net::ntpackage<0xf13e1f40, encrypted_result, data_hash,
                           result_signature, cost_signature>;

  using shu_info_t = ff::util::ntobject<pkey, encrypted_shu_skey,
                                        shu_forward_signature, enclave_hash>;

  define_nt(shu_info, shu_info_t);

  using forward_result_t =
      ff::util::ntobject<shu_info, data_hash, encrypted_result>;

  using offchain_result_package_t =
      ::ff::net::ntpackage<0xf13e1f41, encrypted_result, data_hash,
                           result_signature, cost_signature, pkey,
                           result_encrypt_key>;

  using keymgr_key_package_t =
      ::ff::net::ntpackage<0x314234b9, pkey, sealed_skey, user_id, timestamp>;

  define_nt(access_list_type, uint8_t);
  define_nt(access_data_type, uint8_t);
  using access_item_t =
      ::ff::net::ntpackage<0x71185aff, data, access_data_type>;

  define_nt(access_list, std::vector<access_item_t>);
  using access_list_package_t =
      ::ff::net::ntpackage<0x3c92350f, access_list_type, access_list>;

  using auth_parser_response_pkg_t = ::ff::net::ntpackage<0, succ, msg, data>;
};
} // namespace ypc
