#include "common/limits.h"
#include "corecommon/datahub/package.h"
#include "corecommon/package.h"
#include "stbox/eth/eth_hash.h"
#include "ypc/ntobject_file.h"
#include "ypc/privacy_data_reader.h"
#include "ypc/sealed_file.h"
#include "ypc/sgx/datahub_sgx_module.h"
#include "ypc/status.h"
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <glog/logging.h>
#include <iostream>

using namespace ypc;

typedef ypc::datahub::data_host<ypc::bytes> dhost_t;

uint32_t check_sealed_data(const std::string &sealer_path,
                           const std::string &sealed_file_path,
                           const ypc::bytes &data_hash) {

  std::shared_ptr<ypc::simple_sealed_file> sf;
  try {
    sf = std::make_shared<ypc::simple_sealed_file>(sealed_file_path, true);
  } catch (const std::exception &e) {
    LOG(ERROR) << "error while open sealed file " << sealed_file_path;
    return ypc::sealed_file_cannot_open;
  }

  std::string enclave_file = sealer_path;

  std::shared_ptr<datahub_sgx_module> sm;
  try {
    sm = std::make_shared<datahub_sgx_module>(enclave_file.c_str());
  } catch (const std::exception &e) {
    LOG(ERROR) << "error while open datahub enclave file " << enclave_file;
    return ypc::datahub_enclave_file_cannot_open;
  };

  sm->begin_encrypt_sealed_data();
  ypc::memref item_data;
  std::cout << "checking data hash...." << std::endl;
  while (sf->next_item(item_data)) {
    uint32_t len =
        sm->get_encrypted_sealed_data_size(item_data.data(), item_data.size());
    ypc::bytes encrypted_data(len);
    sm->encrypt_sealed_data(item_data.data(), item_data.size(),
                            encrypted_data.data(), len);
  }
  std::cout << "done check data hash" << std::endl;
  sm->end_encrypt_sealed_data();

  ypc::bytes credential;
  sm->get_encrypted_data_credential(credential);
  auto t =
      ypc::make_package<dhost_t::credential_package_t>::from_bytes(credential);
  if (data_hash == t.get<dhost_t::data_hash>()) {
    std::cout << "correct data hash" << std::endl;
    return ypc::success;
  } else {
    std::cout << "Expect " << data_hash << ", got "
              << t.get<dhost_t::data_hash>() << std::endl;
    return ypc::check_data_hash_fail;
  }
}
