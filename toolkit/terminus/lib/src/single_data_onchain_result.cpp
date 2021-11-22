#include "ypc/terminus/single_data_onchain_result.h"
#include "common/crypto_prefix.h"
#include "common/param_id.h"

namespace ypc {
namespace terminus {
single_data_onchain_result::single_data_onchain_result(crypto_pack *crypto)
    : interaction_base(crypto) {}

single_data_onchain_result::response_t
single_data_onchain_result::generate_response(const bytes &param,
                                              const bytes &tee_pub_key,
                                              const bytes &data_hash,
                                              const bytes &enclave_hash,
                                              const bytes &private_key) {

  ypc::bytes encrypted_param = m_crypto->ecc_encrypt(
      param, tee_pub_key, ypc::utc::crypto_prefix_arbitrary);
  if (encrypted_param.size() == 0) {
    LOG(ERROR) << "encrypt param failed";
    return response_t(ypc::bytes(), ypc::bytes());
  }

  uint32_t msg_id = param_id::PRIVATE_KEY;
  ypc::bytes to_sign_message = ypc::bytes((uint8_t *)&msg_id, sizeof(msg_id)) +
                               encrypted_param + tee_pub_key + enclave_hash;

  auto sig = m_crypto->sign_message(to_sign_message, private_key);

  return response_t(encrypted_param, sig);
}

ypc::bytes
single_data_onchain_result::decrypt_result(const bytes &result,
                                           const bytes &private_key) {
  return m_crypto->ecc_decrypt(result, private_key,
                               ypc::utc::crypto_prefix_arbitrary);
}
} // namespace terminus
} // namespace ypc
