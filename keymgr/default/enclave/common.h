#pragma once

#include "ekeymgr_t.h" /* print_string */
#include <stbox/tsgx/channel/dh_session.h>
#include <ypc_t/ecommon/package.h>

uint32_t load_and_check_key_pair(const uint8_t *pkey, uint32_t pkey_size,
                                 stbox::bytes &skey);
uint32_t load_key_pair_if_not_exist(uint8_t *pkey_ptr, uint32_t pkey_size,
                                    uint8_t *skey_ptr, uint32_t *skey_size);

stbox::bytes handle_data_usage_license_pkg(
    stbox::dh_session *context,
    const request_extra_data_usage_license_pkg_t &pkg);

stbox::bytes handle_extra_data_pkg(stbox::dh_session *context,
                                   const request_extra_data_pkg_t &pkg);
