/*
 * Copyright (C) 2011-2020 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


// Enclave1.cpp : Defines the exported functions for the .so application
#include "EnclaveInitiator_t.h"
#include "sgx_dh.h"
#include "sgx_eid.h"
#include "sgx_utils.h"
#include "stbox/stx_common.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include <stdio.h> /* vsnprintf */
#include <string>

#define RESPONDER_PRODID 0

// This is hardcoded responder enclave's MRSIGNER for demonstration purpose. The
// content aligns to responder enclave's signing key
sgx_measurement_t g_responder_mrsigner = {
    {0x83, 0xd7, 0x19, 0xe7, 0x7d, 0xea, 0xca, 0x14, 0x70, 0xf6, 0xba,
     0xf6, 0x2a, 0x4d, 0x77, 0x43, 0x03, 0xc8, 0x99, 0xdb, 0x69, 0x02,
     0x0f, 0x9c, 0x70, 0xee, 0x1d, 0xfc, 0x08, 0xc7, 0xce, 0x9e}};

using namespace stbox;
/* Function Description:
 *   This is to verify peer enclave's identity.
 * For demonstration purpose, we verify below points:
 *   1. peer enclave's MRSIGNER is as expected
 *   2. peer enclave's PROD_ID is as expected
 *   3. peer enclave's attribute is reasonable: it's INITIALIZED'ed enclave; in
 *non-debug build configuraiton, the enlave isn't loaded with enclave debug
 *mode.
 **/
stbox::stx_status verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity)
    return stbox::stx_status::invalid_parameter_error;

  // check peer enclave's MRSIGNER
  if (memcmp((uint8_t *)&peer_enclave_identity->mr_signer,
             (uint8_t *)&g_responder_mrsigner, sizeof(sgx_measurement_t)))
    return stbox::stx_status::enclave_trust_error;

  // check peer enclave's product ID and enclave attribute (should be
  // INITIALIZED'ed)
  if (peer_enclave_identity->isv_prod_id != RESPONDER_PRODID ||
      !(peer_enclave_identity->attributes.flags & SGX_FLAGS_INITTED))
    return stbox::stx_status::enclave_trust_error;

    // check the enclave isn't loaded in enclave debug mode, except that the
    // project is built for debug purpose
#if defined(NDEBUG)
    // if (peer_enclave_identity->attributes.flags & SGX_FLAGS_DEBUG)
    // return ENCLAVE_TRUST_ERROR;
#endif
  return stbox::stx_status::success;
}

/* Function Description:
 *   This is ECALL routine to create ECDH session.
 *   When it succeeds to create ECDH session, the session context is saved in g_session.
 * */
std::unique_ptr<stbox::dh_session_initiator> dh_init_session(nullptr);
uint32_t create_session() {
  try {
    if (!dh_init_session) {
      dh_init_session = std::unique_ptr<stbox::dh_session_initiator>(
          new stbox::dh_session_initiator(
              stbox::ocall_cast<uint32_t>(session_request_ocall),
              stbox::ocall_cast<uint32_t>(exchange_report_ocall),
              stbox::ocall_cast<uint32_t>(send_request_ocall),
              stbox::ocall_cast<uint32_t>(end_session_ocall)));
      dh_init_session->set_verify_peer(verify_peer_enclave_trust);
    }
    return static_cast<uint32_t>(dh_init_session->create_session());

  } catch (std::exception &e) {
    printf("create_session got exception %s\n", e.what());
    return static_cast<uint32_t>(stbox::stx_status::error_unexpected);
  }
}

/* Function Description:
 *   This is ECALL routine to transfer message with ECDH peer
 * */
uint32_t request_forward(uint32_t msg_id) {
  stbox::stx_status ke_status = stbox::stx_status::success;
  char *marshalled_inp_buff;
  size_t marshalled_inp_buff_len;
  char *out_buff;
  size_t out_buff_len = 0;
  size_t max_out_buff_size;

  // it's assumed the maximum payload size in response message is 1024 bytes,
  // it's for demontration purpose
  max_out_buff_size = 1024;

  // request message
  std::string request_msg((const char *)&msg_id, sizeof(uint32_t));
  marshalled_inp_buff = (char *)request_msg.c_str();
  marshalled_inp_buff_len = request_msg.size();

  // Core Reference Code function
  if (!dh_init_session) {
    return static_cast<uint32_t>(stbox::stx_status::error_unexpected);
  }
  ke_status = dh_init_session->send_request_recv_response(
      marshalled_inp_buff, marshalled_inp_buff_len, max_out_buff_size,
      &out_buff, &out_buff_len);
  printf("response: %s\n", out_buff);
  if (ke_status != stbox::stx_status::success) {
    return (uint32_t)ke_status;
  }
  return (uint32_t)stbox::stx_status::success;
}


/* Function Descriptin:
 *   This is ECALL interface to close secure session*/
uint32_t close_session() {
  if (!dh_init_session) {
    return static_cast<uint32_t>(stbox::stx_status::error_unexpected);
  }
  return static_cast<uint32_t>(dh_init_session->close_session());
}

