#include "stbox/tsgx/crypto/seal_sgx.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/log.h"
#include <sgx_tcrypto.h>
#include <sgx_trts.h>
#include <sgx_tseal.h>

#define AAD_MAC_TEXT_LEN 64
static char aad_mac_text[AAD_MAC_TEXT_LEN] = "tech.yeez.key.manager";

namespace stbox {
namespace crypto {

uint32_t
raw_device_sealer<intel_sgx>::get_sealed_data_size(uint32_t data_size) {
  return sgx_calc_sealed_data_size(strlen(aad_mac_text), data_size);
}
uint32_t raw_device_sealer<intel_sgx>::seal_data(const uint8_t *data,
                                                 uint32_t data_size,
                                                 uint8_t *sealed_data,
                                                 uint32_t sealed_size) {

  sgx_status_t se_ret = sgx_seal_data(
      (const uint32_t)strlen(aad_mac_text), (const uint8_t *)aad_mac_text,
      (const uint32_t)data_size, data, (const uint32_t)sealed_size,
      (sgx_sealed_data_t *)sealed_data);
  return se_ret;
}

uint32_t raw_device_sealer<intel_sgx>::get_unsealed_data_size(
    const uint8_t *sealed_data, uint32_t sealed_data_size) {
  uint32_t skey_size =
      sgx_get_encrypt_txt_len((const sgx_sealed_data_t *)sealed_data);
  return skey_size;
}

uint32_t raw_device_sealer<intel_sgx>::unseal_data(const uint8_t *sealed_data,
                                                   uint32_t sealed_data_size,
                                                   uint8_t *data,
                                                   uint32_t data_size) {
  uint32_t aad_mac_len =
      sgx_get_add_mac_txt_len((const sgx_sealed_data_t *)sealed_data);
  if (aad_mac_len == 0xFFFFFFFF) {
    LOG(ERROR) << "sgx_get_add_mac_txt_len returns unexpected ";
    return ecc_sgx_get_add_mac_txt_len_unexpected;
  }
  uint32_t unseal_size =
      sgx_get_encrypt_txt_len((const sgx_sealed_data_t *)sealed_data);
  if (unseal_size != data_size) {
    LOG(ERROR) << "invalid unseal size: " << unseal_size << ", expect "
               << data_size;
    return stbox::stx_status::ecc_invalid_unseal_size;
  }
  bytes mac_text(aad_mac_len);

  auto se_ret =
      sgx_unseal_data((const sgx_sealed_data_t *)sealed_data, mac_text.data(),
                      (uint32_t *)&aad_mac_len, data, &unseal_size);
  if (se_ret) {
    LOG(ERROR) << "sgx_unseal_data returns: " << se_ret;
    return se_ret;
  }

  auto t = memcmp(mac_text.data(), aad_mac_text, aad_mac_len);
  if (t != 0) {
    LOG(ERROR) << "mac mismatch, got: " << mac_text
               << ", expect: " << aad_mac_text;
    return stbox::stx_status::ecc_sgx_invalid_aad_mac_text;
  }
  if (unseal_size != data_size) {
    LOG(ERROR) << "invalid unseal size: " << unseal_size << ", expect "
               << data_size;
    return stbox::stx_status::ecc_invalid_unseal_size;
  }
  return se_ret;
}
}
} // namespace stbox
