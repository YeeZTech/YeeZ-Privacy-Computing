#include "ypc/terminus/interaction.h"

namespace ypc {
namespace terminus {
class single_data_onchain_result : public interaction_base {
public:
  single_data_onchain_result(crypto_pack *crypto);

  bytes generate_request(const bytes &param, const bytes &private_key);

  bytes decrypt_result(const bytes &result, const bytes &private_key);
};
} // namespace terminus
} // namespace ypc
