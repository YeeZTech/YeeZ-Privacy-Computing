#include "common.h"
#include "datahub_enclave_t.h"
#include "sgx_trts.h"
#include "sgx_tseal.h"
#include "stbox/scope_guard.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/channel/dh_session_responder.h"
#include "stbox/tsgx/log.h"
#include "ypc_t/ecommon/package.h"
#include "ypc_t/ecommon/signer_verify.h"

using stx_status = stbox::stx_status;
char aad_mac_text[64] = "tech.yeez.file.sealer";
using scope_guard = stbox::scope_guard;
using namespace stbox;

uint32_t get_sealed_data_size(uint32_t encrypt_data_size) {
  return sgx_calc_sealed_data_size((uint32_t)strlen(aad_mac_text),
                                   encrypt_data_size);
}

sgx_status_t seal_file_data(uint8_t *encrypt_data, uint32_t in_size,
                            uint8_t *sealed_blob, uint32_t data_size) {
  uint32_t sealed_data_size =
      sgx_calc_sealed_data_size((uint32_t)strlen(aad_mac_text), in_size);
  if (sealed_data_size == UINT32_MAX) {
    LOG(ERROR) << "sealed_data_size is UINT32_MAX, which is unexpected";
    return SGX_ERROR_UNEXPECTED;
  }
  if (sealed_data_size > data_size) {
    LOG(ERROR) << "allocated buffer size is " << data_size
               << ", expected size: " << sealed_data_size;
    return SGX_ERROR_INVALID_PARAMETER;
  }

  uint8_t *temp_sealed_buf = (uint8_t *)malloc(sealed_data_size);
  if (temp_sealed_buf == NULL) {
    LOG(ERROR) << "allocating memory with size " << sealed_data_size
               << " returns NULL";
    return SGX_ERROR_OUT_OF_MEMORY;
  }
  sgx_status_t err = sgx_seal_data((uint32_t)strlen(aad_mac_text),
                                   (const uint8_t *)aad_mac_text, in_size,
                                   (uint8_t *)encrypt_data, sealed_data_size,
                                   (sgx_sealed_data_t *)temp_sealed_buf);
  if (err == SGX_SUCCESS) {
    // Copy the sealed data to outside buffer
    memcpy(sealed_blob, temp_sealed_buf, sealed_data_size);
  } else {
    LOG(ERROR) << "sgx_seal_data returns " << status_string(err);
  }

  free(temp_sealed_buf);
  return err;
}

size_t unsealed_data_len(const uint8_t *sealed_blob, size_t data_size) {
  uint32_t decrypt_data_len =
      sgx_get_encrypt_txt_len((const sgx_sealed_data_t *)sealed_blob);
  return static_cast<size_t>(decrypt_data_len);
}

sgx_status_t unseal_data(const uint8_t *sealed_blob, size_t data_size,
                         uint8_t *raw_data, size_t raw_size) {
  uint32_t mac_text_len =
      sgx_get_add_mac_txt_len((const sgx_sealed_data_t *)sealed_blob);
  uint32_t decrypt_data_len =
      sgx_get_encrypt_txt_len((const sgx_sealed_data_t *)sealed_blob);
  if (mac_text_len == UINT32_MAX || decrypt_data_len == UINT32_MAX) {

    LOG(ERROR) << "(mac_text_len == UINT32_MAX || decrypt_data_len == "
                  "UINT32_MAX), which is unexpected";
    return SGX_ERROR_UNEXPECTED;
  }
  if (mac_text_len > data_size || decrypt_data_len > data_size) {
    LOG(ERROR) << "(mac_text_len > data_size || decrypt_data_len > data_size), "
                  "which is invalid";
    return SGX_ERROR_INVALID_PARAMETER;
  }

  if (decrypt_data_len != raw_size) {
    LOG(ERROR) << "(decrypt_data_len != raw_size), which is invalid";
    return SGX_ERROR_INVALID_PARAMETER;
  }
  if (!raw_data) {
    LOG(ERROR) << "(!raw_data), which is invalid";
    return SGX_ERROR_INVALID_PARAMETER;
  }

  uint8_t *de_mac_text = (uint8_t *)malloc(mac_text_len);
  if (de_mac_text == NULL) {
    LOG(ERROR) << "allocating memory with size " << mac_text_len
               << " returns NULL";
    return SGX_ERROR_OUT_OF_MEMORY;
  }
  uint8_t *decrypt_data = raw_data;
  scope_guard _de([&de_mac_text]() {
    if (de_mac_text) {
      free(de_mac_text);
      de_mac_text = NULL;
    }
  });

  sgx_status_t ret =
      sgx_unseal_data((const sgx_sealed_data_t *)sealed_blob, de_mac_text,
                      &mac_text_len, decrypt_data, &decrypt_data_len);
  if (ret != SGX_SUCCESS) {
    free(decrypt_data);
    return ret;
  }

  if (memcmp(de_mac_text, aad_mac_text, strlen(aad_mac_text))) {
    free(decrypt_data);
    decrypt_data = NULL;
    LOG(ERROR) << "invalid mac_text";
    ret = SGX_ERROR_UNEXPECTED;
  }

  return ret;
}

std::shared_ptr<stbox::dh_session_responder> dh_resp_session(nullptr);
std::shared_ptr<ypc::nt<stbox::bytes>::access_list_package_t>
    access_control_policy;

uint32_t set_access_control_policy(uint8_t *policy, uint32_t in_size) {
  try {
    *access_control_policy = ypc::make_package<
        ypc::nt<stbox::bytes>::access_list_package_t>::from_bytes(policy,
                                                                  in_size);
  } catch (const std::exception &e) {
    LOG(ERROR) << "error when make_package::from_bytes " << e.what();
    return stbox::stx_status::invalid_parameter;
  }
  return stbox::stx_status::success;
}

stbox::stx_status verify_peer_enclave_trust(
    sgx_dh_session_enclave_identity_t *peer_enclave_identity) {
  if (!peer_enclave_identity) {
    LOG(ERROR) << "verify peer enclave failed";
    return stbox::stx_status::invalid_parameter_error;
  }
  if (!ypc::is_certified_signer(peer_enclave_identity, access_control_policy)) {
    return stbox::stx_status::enclave_trust_error;
  }
  return stbox::stx_status::success;
}

stbox::bytes get_data_and_unseal(const uint8_t *data_buf, size_t data_len,
                                 stbox::dh_session *context) {
  uint32_t type_id;
  ::ff::net::deserialize((const char *)data_buf, type_id);
  if (type_id != request_data_item) {
    LOG(ERROR) << "invalid type id " << type_id;

    char buf[256];
    ctrl_pkg_t p;
    sgx_marshaler m(buf, 256, sgx_marshaler::serializer);
    p.arch(m);
    size_t len = m.get_length();
    return stbox::bytes(buf, len);
  } else {
    uint8_t *t_sealed_data;
    uint32_t t_sealed_data_len;
    stx_status ret = static_cast<stx_status>(stbox::ocall_cast<uint32_t>(
        next_sealed_item_data)(&t_sealed_data, &t_sealed_data_len));

    if (ret != stx_status::success) {
      return stbox::bytes();
    }

    uint8_t *sealed_data = (uint8_t *)malloc(t_sealed_data_len);
    scope_guard _s([&sealed_data]() {
      if (sealed_data) {
        free(sealed_data);
        sealed_data = nullptr;
      }
    });

    uint32_t sealed_data_len = t_sealed_data_len;
    // We need move the sealed data from untrusted memory to trusted memory
    memcpy(sealed_data, t_sealed_data, sealed_data_len);

    //! this memory is allocated in next_sealed_item_data, so we need to
    //! deallocate it
    stbox::ocall_cast<void>(free_sealed_item_data)(t_sealed_data);

    if (ret != stx_status::success) {
      char buf[256];
      ctrl_pkg_t p;
      sgx_marshaler m(buf, 256, sgx_marshaler::serializer);
      p.arch(m);
      size_t len = m.get_length();
      return stbox::bytes(buf, len);
    } else {
      // unseal data
      stbox::bytes raw_str(unsealed_data_len(sealed_data, sealed_data_len));
      size_t raw_size = raw_str.size();
      sgx_status_t ut =
          unseal_data(sealed_data, sealed_data_len,
                      reinterpret_cast<uint8_t *>(&raw_str[0]), raw_size);
      if (ut != SGX_SUCCESS) {
        LOG(ERROR) << "unseal failed: " << status_string(ut);
        return stbox::bytes();
      }
      response_pkg_t p;
      p.template set<ypc::nt<stbox::bytes>::data>(std::move(raw_str));
      sgx_marshaler lm(sgx_marshaler::length_retriver);
      p.arch(lm);
      stbox::bytes ret(lm.get_length());

      sgx_marshaler m((char *)&ret[0], ret.size(), sgx_marshaler::serializer);
      p.arch(m);
      return ret;
    }
  }
}

uint32_t session_request(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
      dh_resp_session->set_verify_peer(verify_peer_enclave_trust);
    }
    return static_cast<uint32_t>(
        dh_resp_session->session_request(dh_msg1, session_id));
  } catch (std::exception &e) {
    LOG(ERROR) << "session_request get exception: " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}
uint32_t exchange_report(sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3,
                         uint32_t session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
    }
    return static_cast<uint32_t>(
        dh_resp_session->exchange_report(dh_msg2, dh_msg3, session_id));
  } catch (std::exception &e) {
    LOG(ERROR) << "exchange_report get exception " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}
uint32_t generate_response(secure_message_t *req_message,
                           size_t req_message_size, size_t max_payload_size,
                           secure_message_t *resp_message,
                           size_t resp_message_size, uint32_t session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
    }
    return static_cast<uint32_t>(dh_resp_session->generate_response(
        req_message, req_message_size, get_data_and_unseal, max_payload_size,
        resp_message, resp_message_size, session_id));
  } catch (std::exception &e) {
    LOG(ERROR) << "generate_response get exception " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}
uint32_t end_session(uint32_t session_id) {
  try {
    if (!dh_resp_session) {
      dh_resp_session = std::make_shared<stbox::dh_session_responder>();
    }
    return static_cast<uint32_t>(dh_resp_session->end_session(session_id));
  } catch (std::exception &e) {
    LOG(ERROR) << "end_session get exception " << e.what();
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
}
