#include "test_ypc_module.h"
#include <iostream>

int main(int argc, char *argv[]) {
  ypc::hex_bytes skey_hex(
      "3908a1b53ef489f2e8379298256112c4146475e86ace325c0a4be72b1d7a5043");
  ypc::bytes skey = skey_hex.as<ypc::bytes>();
  ypc::bytes pkey =
      ypc::hex_bytes(
          "5d7ee992f48ffcdb077c2cb57605b602bd4029faed3e91189c7fb9fccc72771e4"
          "5b7aa166766e2ad032d0a195372f5e2d20db792901d559ab0d2bfae10ecea97")
          .as<ypc::bytes>();

  ypc::bytes cipher = ypc::hex_bytes("af4145ab19e5a354c2118032d4a6ca81ac4ddf1ff"
                                     "cd0e227cd648fb1701d95f917e31481"
                                     "e38d832d38f0ffbbba80228ab1ee9c05e88f997ca"
                                     "e4354677f4fe0b9dce23ca07309cddc"
                                     "32dd0997517aec00687314")
                          .as<ypc::bytes>();

  ypc::bytes result("a result here");

  uint64_t cost = 12876;
  ypc::bytes enclave_hash =
      ypc::hex_bytes(
          "3e0d3a43f4f45ba7a1234759c2ffa4028a44599d4ab29bec532bd2057c0f9141")
          .as<ypc::bytes>();
  ypc::bytes data_hash = enclave_hash;

  ypc::bytes encrypted_res;
  ypc::bytes res_sig;
  ypc::bytes cost_sig;

  test_ypc_sgx_module m("../lib/test_ypc_enclave.signed.so");
  m.get_encrypted_result_and_signature(cipher, enclave_hash, result, skey,
                                       data_hash, cost, encrypted_res, res_sig,
                                       cost_sig);
  std::cout << "encrypted result : " << encrypted_res << std::endl;
  std::cout << "result signature: " << res_sig << std::endl;
  std::cout << "cost signature: " << cost_sig << std::endl;
  return 0;
}
