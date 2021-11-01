#include "common/limits.h"
#include "header.h"
#include "stbox/eth/eth_hash.h"
#include "ypc/ntobject_file.h"
#include "ypc/privacy_data_reader.h"
#include "ypc/sealed_file.h"
#include "ypc/sgx/datahub_sgx_module.h"
#include "ypc/sha.h"
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
using namespace ypc;

int gen_host_data(datahub_sgx_module &sm, ypc::simple_sealed_file &sf,
                  const std::string &encrypted_data_path,
                  const std::string &credential_path) {
  // m_sf = std::shared_ptr<ypc::internal::sealed_file_base>(
  // new ypc::simple_sealed_file(m_sealfile_path, true));

  LOG(INFO) << "start generate host data";
  ypc::simple_sealed_file output(encrypted_data_path, false);
  sm.begin_encrypt_sealed_data();
  ypc::memref item_data;
  while (sf.next_item(item_data)) {
    uint32_t len =
        sm.get_encrypted_sealed_data_size(item_data.data(), item_data.size());
    ypc::bytes encrypted_data(len);
    sm.encrypt_sealed_data(item_data.data(), item_data.size(),
                           encrypted_data.data(), len);

    output.write_item(encrypted_data);
  }
  sm.end_encrypt_sealed_data();
  LOG(INFO) << "end generate host data";

  ypc::bytes credential;
  sm.get_encrypted_data_credential(credential);
  LOG(INFO) << "got credential data";
  std::ofstream cf(credential_path, std::ios::out | std::ios::binary);
  if (cf.is_open()) {
    cf.write((const char *)credential.data(), credential.size());
  } else {
    std::cout << "cannot open credential file: " << credential_path
              << std::endl;
    return -1;
  }

  return 0;
}

int gen_data_usage_license(datahub_sgx_module &sm,
                           const std::string &credential_path,
                           const ypc::bytes &encrypted_param,
                           const ypc::bytes &enclave_hash,
                           const ypc::bytes &pkey4v, const ypc::bytes &tee_pkey,
                           const std::string &license_path) {
  std::ifstream _if(credential_path, std::ios::in | std::ios::binary);
  _if.seekg(0, std::ios::end);
  std::fstream::pos_type size = _if.tellg();
  ypc::bytes credential(size);
  _if.seekg(0, std::ios::beg);
  _if.read((char *)credential.data(), credential.size());
  ypc::bytes license;
  sm.generate_data_usage_license(credential, encrypted_param, enclave_hash,
                                 pkey4v, tee_pkey, license);
  std::ofstream of(license_path, std::ios::out | std::ios::binary);
  ypc::hex_bytes hex_license = license.as<ypc::hex_bytes>();
  of.write((const char *)hex_license.data(), hex_license.size());
  return 0;
}

int hosting_data_main(boost::program_options::variables_map &vm) {

  if (!vm.count("sealer-path")) {
    std::cout << "missing 'sealer-path'" << std::endl;
    return -1;
  }
  std::string enclave_file = vm["sealer-path"].as<std::string>();
  std::string credential_path = vm["credential-path"].as<std::string>();
  if (!vm.count("credential-path")) {
    std::cout << " missing 'credential-path' " << std::endl;
    return -1;
  }
  if (!vm.count("output")) {
    std::cout << "missing 'output' " << std::endl;
    return -1;
  }

  datahub_sgx_module sm(enclave_file.c_str());
  if (vm.count("gen-host-data")) {
    std::string sealed_file_path = vm["sealed-data-url"].as<std::string>();
    ypc::simple_sealed_file sf(sealed_file_path, true);
    std::string encrypted_file_path = vm["output"].as<std::string>();

    return gen_host_data(sm, sf, encrypted_file_path, credential_path);
  } else if (vm.count("gen-request-license")) {
    std::string license_path = vm["output"].as<std::string>();
    ypc::bytes encrypted_param =
        ypc::hex_bytes(vm["encrypted-param"].as<std::string>())
            .as<ypc::bytes>();
    ypc::bytes enclave_hash =
        ypc::hex_bytes(vm["enclave-hash"].as<std::string>()).as<ypc::bytes>();
    ypc::bytes pkey4v =
        ypc::hex_bytes(vm["pkey4v"].as<std::string>()).as<ypc::bytes>();
    ypc::bytes tee_pkey =
        ypc::hex_bytes(vm["tee-pkey"].as<std::string>()).as<ypc::bytes>();

    return gen_data_usage_license(sm, credential_path, encrypted_param,
                                  enclave_hash, pkey4v, tee_pkey, license_path);
  }
  return 0;
}
