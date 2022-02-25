#include "ypc_t/ecommon/signer_verify.h"
#include "common/access_policy.h"
#include "stbox/tsgx/log.h"
#include "stbox/tsgx/util.h"

//#define RESPONDER_PRODID 1

namespace ypc {
bool is_certified_signer(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity,
    std::shared_ptr<nt<stbox::bytes>::access_list_package_t> policy) {

  if (/*peer_enclave_identity->isv_prod_id != RESPONDER_PRODID || */
      !(peer_enclave_identity->attributes.flags & SGX_FLAGS_INITTED)) {
    LOG(ERROR) << "SGX_FLAGS_INITTED error";
    return false;
  }

  // check the enclave isn't loaded in enclave debug mode, except that the
  // project is built for debug purpose
#ifdef NDEBUG
#ifndef EDEBUG // disable check in PreRelease mode
  if (peer_enclave_identity->attributes.flags & SGX_FLAGS_DEBUG) {
    LOG(ERROR) << "shouldn't loaded as DEBUG";
    return false;
  }
#endif
#endif

  if (!policy) {
    // default policy: require the peer has the same signer with self signer
    LOG(INFO) << "using default policy";
    auto self_signer = stbox::get_enclave_signer();
    if (self_signer.size() != sizeof(sgx_measurement_t)) {
      LOG(ERROR) << "failed to get self signer";
      return false;
    }
    if (memcmp((uint8_t *)&peer_enclave_identity->mr_signer, self_signer.data(),
               sizeof(sgx_measurement_t))) {
      LOG(ERROR) << "not a trusted signer";
      return false;
    }
    return true;
  }
  using ntt = ypc::nt<stbox::bytes>;
  auto tag = policy->get<ntt::access_list_type>();
  if (tag == ypc::utc::access_policy_whitelist) {
    LOG(INFO) << "using whitelist policy";
    const std::vector<ntt::access_item_t> &list =
        policy->get<ntt::access_list>();
    for (const auto &item : list) {
      auto t = item.get<ntt::access_data_type>();
      auto d = item.get<ntt::data>();
      if (t == ypc::utc::access_policy_enclave) {
        if (d.size() == sizeof(sgx_measurement_t) &&
            0 == memcmp((uint8_t *)&peer_enclave_identity->mr_enclave, d.data(),
                        sizeof(sgx_measurement_t))) {
          return true;
        }
      } else if (t == ypc::utc::access_policy_signer) {
        if (d.size() == sizeof(sgx_measurement_t) &&
            0 == memcmp((uint8_t *)&peer_enclave_identity->mr_signer, d.data(),
                        sizeof(sgx_measurement_t))) {
          return true;
        }
      } else {
        LOG(ERROR) << "unexpect access_data_type " << t;
        return false;
      }
    }
    LOG(ERROR) << "enclave not found in whitelist";
    return false;

  } else if (tag == ypc::utc::access_policy_blacklist) {
    LOG(INFO) << "using blacklist policy";
    const std::vector<ntt::access_item_t> &list =
        policy->get<ntt::access_list>();
    for (const auto &item : list) {
      auto t = item.get<ntt::access_data_type>();
      auto d = item.get<ntt::data>();
      if (t == ypc::utc::access_policy_enclave) {
        if (d.size() == sizeof(sgx_measurement_t) &&
            0 == memcmp((uint8_t *)&peer_enclave_identity->mr_enclave, d.data(),
                        sizeof(sgx_measurement_t))) {
          LOG(ERROR) << "found enclave in blacklist";
          return false;
        }
      } else if (t == ypc::utc::access_policy_signer) {
        if (d.size() == sizeof(sgx_measurement_t) &&
            0 == memcmp((uint8_t *)&peer_enclave_identity->mr_signer, d.data(),
                        sizeof(sgx_measurement_t))) {
          LOG(ERROR) << "found enclave in blacklist";
          return false;
        }
      } else {
        LOG(ERROR) << "unexpect access_data_type " << t;
        return false;
      }
    }
    return true;
  }

      return true;
}
} // namespace ypc
