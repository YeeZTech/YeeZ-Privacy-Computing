#include "dummy_forward.h"
#include "keymgr/common/util.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"

uint32_t dummy_forward(keymgr_sgx_module *ksm_ptr, uint32_t msg_id,
                       const std::string &epkey, const std::string &vpkey) {
  uint32_t ret;
  auto b_epkey = stbox::bytes::from_hex(epkey);
  char raw_data[] = "winter is cold!";
  ypc::bref cipher;
  ret = ksm_ptr->encrypt_message(b_epkey.value(), b_epkey.size(),
                                 (uint8_t *)raw_data, sizeof(raw_data), cipher);

  // TODO Suppose to get self hash using sgx tool
  uint8_t self_ehash[] = {0x9f, 0x5e, 0x76, 0x2f, 0x77, 0x56, 0xbf, 0xdb,
                          0xdc, 0x06, 0xbd, 0x7a, 0x1a, 0x16, 0x30, 0x15,
                          0xae, 0x56, 0x2c, 0x8a, 0x05, 0xb1, 0xd1, 0xfb,
                          0x49, 0xff, 0xfd, 0x83, 0x42, 0xf0, 0x5e, 0x99};
  uint32_t all_size =
      sizeof(msg_id) + cipher.len() + b_epkey.size() + sizeof(self_ehash);
  stbox::bytes all(all_size);
  memcpy(all.value(), &msg_id, sizeof(msg_id));
  memcpy(all.value() + sizeof(msg_id), cipher.data(), cipher.len());
  memcpy(all.value() + sizeof(msg_id) + cipher.len(), b_epkey.value(),
         b_epkey.size());
  memcpy(all.value() + sizeof(msg_id) + cipher.len() + b_epkey.size(),
         self_ehash, sizeof(self_ehash));

  auto b_vpkey = stbox::bytes::from_hex(vpkey);
  uint32_t sealed_size = ksm_ptr->get_secp256k1_sealed_private_key_size();
  stbox::bytes b_skey(sealed_size);
  ocall_load_key_pair(b_vpkey.value(), b_vpkey.size(), b_skey.value(),
                      sealed_size);
  ypc::bref sig;
  ret = ksm_ptr->sign_message(b_skey.value(), sealed_size, all.value(),
                              all_size, sig);
  ret = ksm_ptr->forward_message(msg_id, cipher.data(), cipher.len(),
                                 b_epkey.value(), b_epkey.size(), self_ehash,
                                 sizeof(self_ehash), b_vpkey.value(),
                                 b_vpkey.size(), sig.data(), sig.len());
  return ret;
}
