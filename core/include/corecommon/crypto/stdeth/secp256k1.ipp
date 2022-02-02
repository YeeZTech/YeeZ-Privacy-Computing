#include "corecommon/crypto/stdeth/secp256k1.h"
#include "common/byte.h"
#include "common/endian.h"
#include "stbox/stx_common.h"
#include "stbox/stx_status.h"
#ifdef YPC_SGX
#include "stbox/tsgx/secp256k1/secp256k1.h"
#include "stbox/tsgx/secp256k1/secp256k1_recovery.h"
#include "stbox/tsgx/log.h"
#else
#include <secp256k1.h>
#include <secp256k1_ecdh.h>
#include <secp256k1_recovery.h>
#include <glog/logging.h>
#endif

namespace ypc{
  namespace crypto{

class secp256k1_context_i {
public:
  typedef ::ypc::utc::bytes<uint8_t, ::ypc::utc::byte_encode::raw_bytes> bytes;
  secp256k1_context_i(){
    m_ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                   SECP256K1_CONTEXT_SIGN);

  }
  virtual ~secp256k1_context_i(){
    secp256k1_context_destroy(m_ctx);
  }

  inline secp256k1_context *ctx() { return m_ctx; }

protected:
  secp256k1_context *m_ctx;
};

std::shared_ptr<secp256k1_context_i> context(nullptr);

secp256k1_context *init_secp256k1_context() {
  if (!context) {
    context =
        std::make_shared<secp256k1_context_i>();
  }
  return context->ctx();
}

uint32_t secp256k1::generate_pkey_from_skey(const uint8_t *skey,
                                                     uint32_t skey_size,
                                                     uint8_t *pkey,
                                                     uint32_t pkey_size) {
  auto ctx = init_secp256k1_context();
  if (!ctx || !skey) {
    LOG(ERROR) << "Context or Secret key or Public key is null";
    return stbox::stx_status::ecc_invalid_ctx_or_skey;
  }
  auto ret = secp256k1_ec_pubkey_create(ctx, (secp256k1_pubkey *)pkey, skey);
  if (!ret) {
    LOG(ERROR) << "Pubkey computation failed: " << ret;
    return stbox::stx_status::ecc_secp256k1_ec_pubkey_create_error;
  }
  ::ypc::utc::change_pubkey_endian((uint8_t *)pkey, sizeof(secp256k1_pubkey));
  return SGX_SUCCESS;
}

uint32_t secp256k1::get_signature_size() {
  return 1 + sizeof(secp256k1_ecdsa_signature) * sizeof(unsigned char);
}
uint32_t secp256k1::sign_message(const uint8_t *skey,
                                          uint32_t skey_size,
                                          const uint8_t *data,
                                          uint32_t data_size, uint8_t *sig,
                                          uint32_t sig_size) {
  if(data_size != 32){
    LOG(ERROR)<<"invalid data size, should be 32";
    return stbox::stx_status::ecc_sign_invalid_data_size;
  }

  auto ctx = init_secp256k1_context();

  sig_size = get_signature_size();

  secp256k1_ecdsa_recoverable_signature rsig;
  auto ret =
      secp256k1_ecdsa_sign_recoverable(ctx, &rsig, data, skey, NULL, NULL);
  if (!ret) {
    LOG(ERROR) << "sign error: " << ret;
    return stbox::stx_status::ecc_secp256k1_ecdsa_sign_recoverable_error;
  }
  int recid;
  ret = secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, sig,
                                                                &recid, &rsig);
  if (!ret) {
    LOG(ERROR) << "serialize sig error: " << ret;
    return stbox::stx_status::ecc_secp256k1_ecdsa_RSSC_error;
  }
  sig[64] = (uint8_t)(recid + 27);
  return stbox::stx_status::success;
}

uint32_t secp256k1::verify_signature(
    const uint8_t *data, uint32_t data_size, const uint8_t *sig,
    uint32_t sig_size, const uint8_t *public_key, uint32_t pkey_size) {

  if(data_size != 32){
    LOG(ERROR)<<"invalid data size, should be 32";
    return stbox::stx_status::ecc_verify_invalid_data_size;
  }
  sgx_status_t se_ret;
  auto ctx = init_secp256k1_context();

  secp256k1_pubkey secp256k1_pkey;
  memcpy(&secp256k1_pkey, public_key, pkey_size);
  ::ypc::utc::change_pubkey_endian((uint8_t *)&secp256k1_pkey,
                                   sizeof(secp256k1_pubkey));

  secp256k1_ecdsa_recoverable_signature rsig;
  se_ret = (sgx_status_t)secp256k1_ecdsa_recoverable_signature_parse_compact(
      ctx, &rsig, sig, *(sig + 64) - 27);
  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdsa_recoverable_signature_parse_compact return "
               << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256k1_ecdsa_sign_recoverable_error;
  }
  secp256k1_ecdsa_signature secp256k1_sig;
  se_ret = (sgx_status_t)secp256k1_ecdsa_recoverable_signature_convert(
      ctx, &secp256k1_sig, &rsig);
  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdsa_recoverable_signature_convert return "
               << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256k1_ecdsa_sign_recoverable_error;
  }

  //auto hash = stbox::eth::msg_hash(data, data_size);
  se_ret = (sgx_status_t)secp256k1_ecdsa_verify(ctx, &secp256k1_sig,
                                                data, &secp256k1_pkey);
  if (!se_ret) {
    LOG(ERROR) << "secp256k1_ecdsa_verify return " << (uint32_t)se_ret;
    return stbox::stx_status::ecc_secp256_ecdsa_verify_error;
  }
  return SGX_SUCCESS;
}

  }
}
