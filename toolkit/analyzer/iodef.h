#pragma once
#include "ypc/core/byte.h"
#include "ypc/core/ntjson.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/corecommon/package.h"

using ntt = ypc::nt<ypc::bytes>;

define_nt(input_data_url, std::string);
define_nt(input_data_hash, ypc::bytes);
define_nt(enclave_hash, ypc::bytes);

define_nt(shu_pkey, ypc::bytes);
typedef ::ff::util::ntobject<shu_pkey, ntt::encrypted_shu_skey,
                             ntt::shu_forward_signature, enclave_hash>
    shu_info_t;

define_nt(shu_info, shu_info_t);
typedef ::ff::util::ntobject<input_data_url, input_data_hash, shu_info,
                             ntt::tag>
    data_item_t;
define_nt(input_data, std::vector<data_item_t>);
define_nt(parser_path, std::string);
define_nt(parser_enclave_hash, ypc::bytes);
define_nt(keymgr_path, std::string);
define_nt(dian_pkey, ypc::bytes);

typedef ::ff::util::ntobject<shu_info, input_data, parser_path, keymgr_path,
                             parser_enclave_hash, dian_pkey, ntt::model,
                             ntt::param>
    input_param_t;

// task graph input param
define_nt(algo_shu_info, shu_info_t);
// input intermediate data define
define_nt(data_shu_infos, std::vector<shu_info_t>);
define_nt(kgt_pkey, ypc::bytes);
typedef ::ff::util::ntobject<kgt_pkey, data_shu_infos> kgt_shu_info_t;
define_nt(kgt_shu_info, kgt_shu_info_t);
typedef ::ff::util::ntobject<input_data_url, input_data_hash, kgt_shu_info,
                             ntt::tag>
    intermediate_data_item_t;
define_nt(input_intermediate_data, std::vector<intermediate_data_item_t>);
// task graph input param define
typedef ::ff::util::ntobject<shu_info, algo_shu_info, input_intermediate_data,
                             parser_path, keymgr_path, parser_enclave_hash,
                             dian_pkey, ntt::model, ntt::param>
    tg_input_param_t;
