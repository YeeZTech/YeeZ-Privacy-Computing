#include "ypc/terminus/single_data_onchain_result.h"
#include "common/crypto_prefix.h"

namespace ypc {
namespace terminus {
single_data_onchain_result::single_data_onchain_result(crypto_pack *crypto)
    : interaction_base(crypto) {}

bytes single_data_onchain_result::generate_request(const bytes &param,
                                                   const bytes &pubkey) {

  ypc::bytes encrypted_param =
      m_crypto->ecc_encrypt(param, pubkey, ypc::utc::crypto_prefix_arbitrary);

  if (encrypted_param.size() == 0) {
    LOG(ERROR) << "encrypt param failed";
    return ypc::bytes();
  }
  return encrypted_param;
}

ypc::bytes
single_data_onchain_result::decrypt_result(const bytes &result,
                                           const bytes &private_key) {
  return m_crypto->ecc_decrypt(result, private_key,
                               ypc::utc::crypto_prefix_arbitrary);
}
} // namespace terminus
} // namespace ypc
