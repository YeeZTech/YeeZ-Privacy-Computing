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

#include "dummy_forward.h"
#include "dummy_sgx_module.h"
#include "keymgr/default/keymgr_sgx_module.h"
#include "sgx_eid.h"
#include "sgx_urts.h"
#include "sgx_utils.h"
#include <memory>
#include <stdio.h>

#define ENCLAVE_INITIATOR_NAME "../lib/dummy_initiator.signed.so"
#define ENCLAVE_RESPONDER_NAME "../lib/keymgr.signed.so"


std::unique_ptr<keymgr_sgx_module> ksm_ptr(nullptr);

int main(int argc, char *argv[]) {
  sgx_status_t status;

  (void)argc;
  (void)argv;

  // load initiator and responder enclaves
  std::string initiator_path = ENCLAVE_INITIATOR_NAME;
  std::string responder_path = ENCLAVE_RESPONDER_NAME;
  auto dsm_ptr = std::make_unique<dummy_sgx_module>(initiator_path.c_str());
  ksm_ptr = std::make_unique<keymgr_sgx_module>(responder_path.c_str());
  printf("succeed to load enclaves.\n");

  // create ECDH session using initiator enclave, it would create session with
  // responder enclave
  status = (sgx_status_t)dsm_ptr->create_session();
  printf("succeed to establish secure channel.\n");

  // test message exchanging between initiator enclave and responder enclave
  status = (sgx_status_t)dummy_forward(ksm_ptr.get(), 100, argv[1], argv[2]);
  status = (sgx_status_t)dsm_ptr->request_forward(100);
  printf("Succeed to exchange secure message...\n");

  // close ECDH session
  status = (sgx_status_t)dsm_ptr->close_session();
  printf("Succeed to close Session...\n");

  return 0;
}

