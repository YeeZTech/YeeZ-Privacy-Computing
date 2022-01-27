#include "./test_ypc_module.h"
#include "common/crypto_prefix.h"
#include "enclave_u.h"
#include "sgx_dh.h"
#include "sgx_eid.h"
#include "sgx_error.h"
#include "stbox/eth/util.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_cdef.h"
#include "stbox/usgx/sgx_module.h"
#include "ypc/byte.h"
#include "ypc/ref.h"
#include <iostream>

test_ypc_sgx_module::test_ypc_sgx_module(const char *mod_path)
    : stbox::sgx_module(mod_path) {}

uint32_t test_ypc_sgx_module::get_encrypted_result_and_signature(
    const ypc::bytes &encrypted_param, const ypc::bytes &enclave_hash,
    const ypc::bytes &result, const ypc::bytes &private_key,
    const ypc::bytes &data_hash, uint64_t cost, ypc::bytes &encrypted_res,
    ypc::bytes &res_sig, ypc::bytes &cost_sig) {
  auto len =
      ecall<uint32_t>(::get_encrypt_message_size_with_prefix, result.size());
  res_sig = ypc::bytes(65);
  cost_sig = ypc::bytes(65);
  encrypted_res = ypc::bytes(len);
  auto t = ecall<uint32_t>(
      ::get_encrypted_result_and_signature, (uint8_t *)encrypted_param.data(),
      encrypted_param.size(), (uint8_t *)enclave_hash.data(),
      enclave_hash.size(), (uint8_t *)result.data(), result.size(),
      (uint8_t *)private_key.data(), private_key.size(),
      (uint8_t *)data_hash.data(), data_hash.size(), cost,
      (uint8_t *)encrypted_res.data(), encrypted_res.size(),
      (uint8_t *)res_sig.data(), res_sig.size(), (uint8_t *)cost_sig.data(),
      cost_sig.size());
  return t;
}

uint32_t test_ypc_sgx_module::seal_data(const ypc::bytes &data,
                                        ypc::bytes &sealed_data) {
  auto len = ecall<uint32_t>(::get_sealed_data_size, data.size());
  sealed_data = ypc::bytes(len);
  auto t = ecall<uint32_t>(::test_seal_data, (uint8_t *)data.data(),
                           data.size(), sealed_data.data(), sealed_data.size());
  return t;
}
