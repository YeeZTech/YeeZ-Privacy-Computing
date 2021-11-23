#include "ypc/terminus/interaction.h"

namespace ypc {
namespace terminus {
class single_data_onchain_result : public interaction_base {
public:
  typedef struct _response {
    inline _response(const ypc::bytes &_encrypted_param,
                     const ypc::bytes &_encrypted_skey, const ypc::bytes &_sig)
        : encrypted_param(_encrypted_param), encrypted_skey(_encrypted_skey),
          signature(_sig) {}

    ypc::bytes encrypted_param;
    ypc::bytes encrypted_skey;
    ypc::bytes signature;
  } response_t;

  single_data_onchain_result(crypto_pack *crypto);

  response_t generate_response(const bytes &param, const bytes &tee_pub_key,
                               const bytes &data_hash,
                               const bytes &enclave_hash,
                               const bytes &private_key);

  ypc::bytes decrypt_result(const bytes &result, const bytes &private_key);
};
} // namespace terminus
} // namespace ypc
