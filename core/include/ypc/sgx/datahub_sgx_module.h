#pragma once
#include "sgx_dh.h"
#include "sgx_eid.h"
#include "sgx_error.h"
#include "stbox/tsgx/channel/dh_cdef.h"
#include "stbox/usgx/sgx_module.h"
#include "ypc/byte.h"
#include <string>

namespace ypc {
class datahub_sgx_module : public stbox::sgx_module {
public:
  datahub_sgx_module(const char *mod_path);
  virtual ~datahub_sgx_module();

  uint32_t get_sealed_data_size(uint32_t encrypt_data_size);
  bytes seal_data(const bytes &data);

  uint32_t session_request(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);
  uint32_t exchange_report(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
                           uint32_t session_id);
  uint32_t generate_response(secure_message_t *req_message,
                             size_t req_message_size, size_t max_payload_size,
                             secure_message_t *resp_message,
                             size_t resp_message_size, uint32_t session_id);
  uint32_t end_session(uint32_t session_id);
};
} // namespace ypc
