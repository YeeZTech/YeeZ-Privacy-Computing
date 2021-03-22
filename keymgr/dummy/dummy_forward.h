#pragma once
#include "keymgr/default/keymgr_sgx_module.h"
#include <cstdint>

uint32_t dummy_forward(keymgr_sgx_module *ksm_ptr, uint32_t msg_id,
                       const std::string &epkey, const std::string &vpkey);
