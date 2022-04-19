#include "cmd_line.h"
#include "corecommon/nt_cols.h"

using ntt = ypc::nt<ypc::bytes>;

int gen_relay_result_proof(ypc::terminus::crypto_pack *crypto,
                           const boost::program_options::variables_map &vm) {

  ypc::bytes param = get_param_use_param(vm);
  ypc::bytes private_key = get_param_privatekey(vm);
  ypc::bytes relay_enclave_hash =
      ypc::hex_bytes(vm["relay-enclave-hash"].as<std::string>())
          .as<ypc::bytes>();

  ypc::bytes target_enclave_hash =
      ypc::hex_bytes(vm["target-enclave-hash"].as<std::string>())
          .as<ypc::bytes>();
  ypc::bytes relay_tee_pkey =
      ypc::hex_bytes(vm["relay-tee-pubkey"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes target_tee_pkey =
      ypc::hex_bytes(vm["target-tee-pubkey"].as<std::string>())
          .as<ypc::bytes>();
  auto to_sign_msg = param + relay_enclave_hash + relay_tee_pkey +
                     target_enclave_hash + target_tee_pkey;

  LOG(INFO) << "to sign message: " << to_sign_msg;
  ypc::bytes sig = crypto->sign_message(to_sign_msg, private_key);
  ypc::bytes pkey = crypto->gen_ecc_public_key_from_private_key(private_key);
  ypc::bytes allowance = sig;

  ntt::forward_target_info_t f;
  f.set<ntt::enclave_hash>(target_enclave_hash);
  f.set<ntt::pkey>(target_tee_pkey);
  f.set<ntt::encrypted_sig>(allowance);

  if (vm.count("output")) {
    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());
    ypc::ntjson::to_json_file(f, output_path);

  } else {
    std::cout << ypc::ntjson::to_json(f) << std::endl;
  }

  return 0;
}
