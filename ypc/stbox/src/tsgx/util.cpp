#include "ypc/stbox/tsgx/util.h"
#include <sgx_report.h>
#include <sgx_utils.h>
namespace stbox {

stbox::bytes get_enclave_signer() {
  stbox::bytes signer(sizeof(sgx_measurement_t));
  memcpy(signer.data(), sgx_self_report()->body.mr_signer.m,
         sizeof(sgx_measurement_t));
  return signer;
}

stbox::bytes get_enclave_hash() {
  stbox::bytes hash(sizeof(sgx_measurement_t));
  memcpy(hash.data(), sgx_self_report()->body.mr_enclave.m,
         sizeof(sgx_measurement_t));
  return hash;
}
} // namespace stbox
