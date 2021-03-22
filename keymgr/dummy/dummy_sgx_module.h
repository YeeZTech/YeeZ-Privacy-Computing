#pragma once
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_cdef.h"
#include "stbox/usgx/sgx_module.h"
#include <sgx_dh.h>
#include <sgx_eid.h>
#include <sgx_error.h>

class dummy_sgx_module : public stbox::sgx_module {
public:
  dummy_sgx_module(const char *mod_path);
  virtual ~dummy_sgx_module();

  uint32_t create_session();
  uint32_t request_forward(uint32_t msg_id);
  uint32_t close_session();
};
