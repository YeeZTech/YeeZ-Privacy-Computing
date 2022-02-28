#include "common/limits.h"
#include "corecommon/datahub/package.h"
#include "corecommon/package.h"
#include "header.h"
#include "stbox/eth/eth_hash.h"
#include "ypc/ntobject_file.h"
#include "ypc/privacy_data_reader.h"
#include "ypc/sealed_file.h"
#include "ypc/sgx/datahub_sgx_module.h"
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
using namespace ypc;

typedef ypc::datahub::data_host<ypc::bytes> dhost_t;

int check_sealed_data(const boost::program_options::variables_map &vm) {

  std::vector<std::string> to_check_opts(
      {"sealer-path", "sealed-data-url", "data-hash"});
  for (auto opt : to_check_opts) {
    if (!vm.count(opt)) {
      std::cout << "missing '" << opt << "'" << std::endl;
      return -1;
    }
  }
  std::string sealed_file_path = vm["sealed-data-url"].as<std::string>();
  ypc::simple_sealed_file sf(sealed_file_path, true);
  std::string enclave_file = vm["sealer-path"].as<std::string>();
  ypc::bytes data_hash =
      ypc::hex_bytes(vm["sealer-path"].as<std::string>()).as<ypc::bytes>();

  datahub_sgx_module sm(enclave_file.c_str());

  sm.begin_encrypt_sealed_data();
  ypc::memref item_data;
  std::cout << "checking data hash...." << std::endl;
  while (sf.next_item(item_data)) {
    uint32_t len =
        sm.get_encrypted_sealed_data_size(item_data.data(), item_data.size());
    ypc::bytes encrypted_data(len);
    sm.encrypt_sealed_data(item_data.data(), item_data.size(),
                           encrypted_data.data(), len);

  }
  std::cout << "done check data hash" << std::endl;
  sm.end_encrypt_sealed_data();

  ypc::bytes credential;
  sm.get_encrypted_data_credential(credential);
  auto t =
      ypc::make_package<dhost_t::credential_package_t>::from_bytes(credential);
  if (data_hash != t.get<dhost_t::data_hash>()) {
    std::cout << "correct data hash" << std::endl;
    return 0;
  } else {
    std::cout << "Expect " << data_hash << ", got "
              << t.get<dhost_t::data_hash>() << std::endl;
    return -1;
  }
}
