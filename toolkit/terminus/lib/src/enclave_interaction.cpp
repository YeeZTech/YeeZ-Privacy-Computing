#include "ypc/terminus/enclave_interaction.h"
#include "common/crypto_prefix.h"

namespace ypc {
namespace terminus {

enclave_interaction::enclave_interaction(crypto_pack *crypto)
    : interaction_base(crypto) {}

bytes enclave_interaction::generate_allowance(const bytes &private_key,
                                              const bytes &param_hash,
                                              const bytes &target_enclave_hash,
                                              const bytes &dian_pkey,
                                              const bytes &dhash) {
  bytes to_sign_data = param_hash + target_enclave_hash + dian_pkey + dhash;

  return m_crypto->sign_message(to_sign_data, private_key);
}

enclave_interaction::forward_info
enclave_interaction::forward_private_key(const bytes &private_key,
                                         const bytes &dian_pkey,
                                         const bytes &enclave_hash) {
  bytes encrypted_skey = m_crypto->ecc_encrypt(private_key, dian_pkey,
                                               ypc::utc::crypto_prefix_forward);
  bytes to_sign_msg = dian_pkey + enclave_hash;
  bytes sig = m_crypto->sign_message(to_sign_msg, private_key);
  return forward_info(encrypted_skey, sig);
}
} // namespace terminus
} // namespace ypc
