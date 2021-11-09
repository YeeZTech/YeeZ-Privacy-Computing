#include "common.h"
#include <common/crypto_prefix.h>
#include <corecommon/datahub/package.h>
#include <stbox/stx_common.h>
#include <stbox/tsgx/crypto/ecc.h>
#include <stbox/tsgx/ocall.h>
#include <unordered_set>

define_nt(data_skey_nt, stbox::bytes);
define_nt(signature_nt, stbox::bytes);
typedef ::ff::util::ntobject<data_skey_nt, signature_nt>
    extra_data_usage_license_t;
std::unordered_map<stbox::bytes, extra_data_usage_license_t>
    extra_data_usage_licenses;
std::unordered_map<stbox::bytes, stbox::bytes> authorized_extra_data;

stbox::bytes tee_pkey;

using ntt = ypc::nt<stbox::bytes>;

uint32_t forward_extra_data_usage_license(
    uint8_t *enclave_pkey, uint32_t epkey_size, uint8_t *_data_hash,
    uint32_t hash_size, uint8_t *usage_license, uint32_t license_size) {

  if (tee_pkey.empty()) {
    tee_pkey = stbox::bytes(enclave_pkey, epkey_size);
  }

  typedef ypc::datahub::data_host<stbox::bytes> dhost_t;
  dhost_t::usage_license_package_t lp;
  ::ff::net::marshaler ar((char *)usage_license, license_size,
                          ::ff::net::marshaler::deseralizer);
  ar.archive(lp);

  stbox::bytes data_hash(_data_hash, hash_size);
  if (extra_data_usage_licenses.find(data_hash) !=
      extra_data_usage_licenses.end()) {
    LOG(ERROR) << "already have usage license for data " << data_hash;
    return static_cast<uint32_t>(stbox::stx_status::error_unexpected);
  }

  stbox::bytes eskey = lp.get<dhost_t::encrypted_skey>();
  stbox::bytes license = lp.get<dhost_t::signature>();

  uint32_t se_ret = SGX_SUCCESS;
  stbox::bytes skey;
  se_ret = load_and_check_key_pair(enclave_pkey, epkey_size, skey);
  if (se_ret) {
    return se_ret;
  }

  uint32_t decrypted_size =
      ::stbox::crypto::get_decrypt_message_size_with_prefix(eskey.size());
  stbox::bytes data_skey(decrypted_size);
  se_ret = (sgx_status_t)::stbox::crypto::decrypt_message_with_prefix(
      skey.data(), skey.size(), eskey.data(), eskey.size(), data_skey.data(),
      data_skey.size(), ::ypc::utc::crypto_prefix_host_data_private_key);

  if (se_ret) {
    LOG(ERROR) << "decrypt_message_with_prefix forward returns " << se_ret;
    return se_ret;
  }

  extra_data_usage_license_t edul;
  edul.set<data_skey_nt>(std::move(data_skey));
  edul.set<signature_nt>(std::move(license));
  extra_data_usage_licenses.insert(std::make_pair(data_hash, edul));
  return se_ret;
}

stbox::bytes handle_data_usage_license_pkg(
    stbox::dh_session *context,
    const request_extra_data_usage_license_pkg_t &pkg) {

  stbox::bytes data_hash = pkg.get<ntt::data_hash>();
  stbox::bytes encrypted_param = pkg.get<ntt::encrypted_param>();
  stbox::bytes enclave_hash((char *)&context->peer_identity().mr_enclave,
                            sizeof(sgx_measurement_t));
  stbox::bytes pkey4v = pkg.get<ntt::pkey4v>();
  stbox::bytes data =
      encrypted_param + enclave_hash + pkey4v + tee_pkey + data_hash;

  auto it = extra_data_usage_licenses.find(data_hash);
  if (it == extra_data_usage_licenses.end()) {
    LOG(WARNING) << "no license";
    return ypc::make_bytes<stbox::bytes>::for_package<
        ack_extra_data_usage_license_pkg_t, ntt::reserve>(0);
  } else {
    auto edul = it->second;
    stbox::bytes data_skey = edul.get<data_skey_nt>();
    stbox::bytes signature = edul.get<signature_nt>();

    stbox::bytes expect_pkey(stbox::crypto::get_secp256k1_public_key_size());
    stbox::crypto::generate_secp256k1_pkey_from_skey(
        data_skey.data(), expect_pkey.data(), expect_pkey.size());
    auto se_ret = (sgx_status_t)verify_signature(
        data.data(), data.size(), signature.data(), signature.size(),
        expect_pkey.data(), expect_pkey.size());
    if (se_ret) {
      LOG(ERROR) << "invalid data usage license for extra data: " << data_hash;
      return ypc::make_bytes<stbox::bytes>::for_package<
          ack_extra_data_usage_license_pkg_t, ntt::reserve>(0);

    } else {
      authorized_extra_data.insert(
          std::make_pair(data_hash + enclave_hash, data_skey));

      return ypc::make_bytes<stbox::bytes>::for_package<
          ack_extra_data_usage_license_pkg_t, ntt::reserve>(1);
    }
  }
}
stbox::bytes handle_extra_data_pkg(stbox::dh_session *context,
                                   const request_extra_data_pkg_t &pkg) {
  stbox::bytes data_hash = pkg.get<ntt::data_hash>();
  stbox::bytes enclave_hash((char *)&context->peer_identity().mr_enclave,
                            sizeof(sgx_measurement_t));
  auto key = data_hash + enclave_hash;
  auto it = authorized_extra_data.find(key);
  if (it == authorized_extra_data.end()) {
    LOG(ERROR) << "unauthroized extra data request";

    return ypc::make_bytes<stbox::bytes>::for_package<ack_extra_data_pkg_t,
                                                      ntt::data>(
        stbox::bytes());
  } else {
    stbox::bytes data_skey = it->second;

    uint32_t r = stbox::ocall_cast<uint32_t>(ocall_read_next_extra_data_item)(
        data_hash.data(), data_hash.size());
    if (r != stbox::stx_status::success) {
      return ypc::make_bytes<stbox::bytes>::for_package<ack_extra_data_pkg_t,
                                                        ntt::data>(
          stbox::bytes());
    }

    uint32_t s =
        stbox::ocall_cast<uint32_t>(ocall_get_next_extra_data_item_size)();
    stbox::bytes edata(s);
    stbox::ocall_cast<uint32_t>(ocall_get_next_extra_data_item_data)(
        edata.data(), edata.size());

    uint32_t decrypted_size =
        ::stbox::crypto::get_decrypt_message_size_with_prefix(edata.size());
    stbox::bytes data(decrypted_size);
    r = (sgx_status_t)::stbox::crypto::decrypt_message_with_prefix(
        data_skey.data(), data_skey.size(), edata.data(), edata.size(),
        data.data(), data.size(), ::ypc::utc::crypto_prefix_host_data);

    if (r != stbox::stx_status::success) {
      LOG(ERROR) << "decrypt data : " << data_hash << " got error " << r;
      return ypc::make_bytes<stbox::bytes>::for_package<ack_extra_data_pkg_t,
                                                        ntt::data>(
          stbox::bytes());
    }

    // LOG(INFO) << "decrypt and got raw data: " << data;
    return ypc::make_bytes<stbox::bytes>::for_package<ack_extra_data_pkg_t,
                                                      ntt::data>(data);
  }
}
