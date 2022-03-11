#include "ypc/terminus/single_data_onchain_result.h"
#include "common/crypto_prefix.h"
#include "common/param_id.h"

namespace ypc {
namespace terminus {
single_data_onchain_result::single_data_onchain_result(crypto_pack *crypto)
    : interaction_base(crypto) {}

single_data_onchain_result::request
single_data_onchain_result::generate_request(const bytes &param,
                                             const bytes &tee_pub_key,
                                             const bytes &data_hash,
                                             const bytes &enclave_hash,
                                             const bytes &private_key) {

  auto pubkey = m_crypto->gen_ecc_public_key_from_private_key(private_key);
  ypc::bytes encrypted_param =
      m_crypto->ecc_encrypt(param, pubkey, ypc::utc::crypto_prefix_arbitrary);

  if (encrypted_param.size() == 0) {
    LOG(ERROR) << "encrypt param failed";
    return request(ypc::bytes(), ypc::bytes(), ypc::bytes());
  }
  ypc::bytes encrypted_skey = m_crypto->ecc_encrypt(
      private_key, tee_pub_key, ypc::utc::crypto_prefix_forward);

  uint32_t msg_id = param_id::PRIVATE_KEY;
  ypc::bytes to_sign_message = ypc::bytes((uint8_t *)&msg_id, sizeof(msg_id)) +
                               private_key + tee_pub_key + enclave_hash;

  auto sig = m_crypto->sign_message(to_sign_message, private_key);

  return request(encrypted_param, encrypted_skey, sig);
}

ypc::bytes
single_data_onchain_result::decrypt_result(const bytes &result,
                                           const bytes &private_key) {
  return m_crypto->ecc_decrypt(result, private_key,
                               ypc::utc::crypto_prefix_arbitrary);
}
} // namespace terminus
} // namespace ypc
