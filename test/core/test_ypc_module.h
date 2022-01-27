#pragma once
#include "common/crypto_prefix.h"
#include "stbox/usgx/sgx_module.h"
#include "ypc/byte.h"
#include <iostream>

using stx_status = stbox::stx_status;
class test_ypc_sgx_module : public stbox::sgx_module {
public:
  test_ypc_sgx_module(const char *mod_path);

  uint32_t get_encrypted_result_and_signature(
      const ypc::bytes &encrypted_param, const ypc::bytes &enclave_hash,
      const ypc::bytes &result, const ypc::bytes &private_key,
      const ypc::bytes &data_hash, uint64_t cost, ypc::bytes &encrypted_res,
      ypc::bytes &res_sig, ypc::bytes &cost_sig);

  uint32_t seal_data(const ypc::bytes &data, ypc::bytes &sealed_data);
};
