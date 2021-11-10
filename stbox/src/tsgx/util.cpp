#include "stbox/tsgx/util.h"
#include <sgx_report.h>
#include <sgx_utils.h>
namespace stbox {

stbox::bytes get_enclave_signer() {
  sgx_report_data_t data;
  uint32_t ret = 0;
  memset(&data.d, 0xff, sizeof data.d);
  sgx_report_t report;
  ret = sgx_create_report(NULL, &data, &report);
  if (ret != SGX_SUCCESS) {
    return stbox::bytes();
  }
  stbox::bytes signer(sizeof(sgx_measurement_t));
  memcpy(signer.data(), report.body.mr_signer.m, sizeof(sgx_measurement_t));
  return signer;
}
} // namespace stbox
