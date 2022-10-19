#include "ypc/terminus/interaction.h"

namespace ypc {
namespace terminus {
class enclave_interaction : public interaction_base {
public:
  typedef struct _forward {
    inline _forward(const ypc::bytes &_encrypted_skey, const ypc::bytes &_sig)
        : encrypted_skey(_encrypted_skey), signature(_sig) {}

    ypc::bytes encrypted_skey;
    ypc::bytes signature;
  } forward_info;

  enclave_interaction(crypto_pack *crypto);

  bytes generate_allowance(const bytes &private_key, const bytes &param_hash,
                           const bytes &target_enclave_hash,
                           const bytes &dian_pkey, const bytes &dhash);

  forward_info forward_private_key(const bytes &private_key,
                                   const bytes &dian_pkey,
                                   const bytes &enclave_hash);
};
} // namespace terminus
} // namespace ypc
