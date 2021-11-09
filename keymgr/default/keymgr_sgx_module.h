#pragma once
#include "sgx_dh.h"
#include "sgx_eid.h"
#include "sgx_error.h"
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "stbox/tsgx/channel/dh_cdef.h"
#include "stbox/usgx/sgx_module.h"
#include "ypc/ref.h"
#include <ypc/byte.h>

using stx_status = stbox::stx_status;
using bref = ypc::bref;
class keymgr_sgx_module : public stbox::sgx_module {
public:
  keymgr_sgx_module(const char *mod_path);
  virtual ~keymgr_sgx_module();

  ///////////
  uint32_t get_secp256k1_sealed_private_key_size();
  uint32_t generate_secp256k1_key_pair(bref &public_key,
                                       bref &sealed_private_key);

  uint32_t sign_message(const uint8_t *sealed_private_key, uint32_t sealed_size,
                        const uint8_t *data, uint32_t data_size, bref &sig);

  uint32_t verify_signature(const uint8_t *data, uint32_t data_size,
                            const uint8_t *sig, uint32_t sig_size,
                            const uint8_t *public_key, uint32_t pkey_size);

  uint32_t encrypt_message(const uint8_t *public_key, uint32_t pkey_size,
                           const uint8_t *data, uint32_t data_size,
                           ypc::bref &cipher);

  uint32_t decrypt_message(const uint8_t *sealed_private_key,
                           uint32_t sealed_size, const uint8_t *cipher,
                           uint32_t cipher_size, bref &data);

  uint32_t forward_private_key(const uint8_t *sealed_private_key,
                               uint32_t sealed_size, const uint8_t *pub_key,
                               uint32_t pkey_size, bref &forward_private_key);

  uint32_t session_request(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);

  uint32_t exchange_report(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
                           uint32_t session_id);
  uint32_t generate_response(secure_message_t *req_message,
                             size_t req_message_size, size_t max_payload_size,
                             secure_message_t *resp_message,
                             size_t resp_message_size, uint32_t session_id);
  uint32_t end_session(uint32_t session_id);

  uint32_t forward_message(uint32_t msg_id, const uint8_t *cipher,
                           uint32_t cipher_size, const uint8_t *epublic_key,
                           uint32_t epkey_size, const uint8_t *ehash,
                           uint32_t ehash_size, const uint8_t *verify_key,
                           uint32_t vpkey_size, const uint8_t *sig,
                           uint32_t sig_size);

  uint32_t
  forward_extra_data_usage_license(const ypc::bytes &enclave_pkey,
                                   const ypc::bytes &data_hash,
                                   const ypc::bytes &data_usage_license);
};
