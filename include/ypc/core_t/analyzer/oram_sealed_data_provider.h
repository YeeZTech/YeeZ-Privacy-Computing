#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/corecommon/ttypes.h"

namespace ypc {
template <typename Crypto>
class oram_sealed_data_provider : public data_source_with_dhash {
  typedef Crypto crypto;
public:
  // TODO:这里需要获取请求参数param_data(加密)和param_private_key
  oram_sealed_data_provider(const stbox::bytes &data_hash, const stbox::bytes &private_key,
                       const stbox::bytes &param_data, const stbox::bytes &param_private_key)
      : data_source_with_dhash(data_hash), m_private_key(private_key) {
    // magic string here, Do Not Change!
    crypto::hash_256(stbox::bytes("Fidelius"), m_actual_data_hash);

    // 解密param_data
    ret = crypto::decrypt_message_with_prefix(param_private_key,
                                           param_data, m_decrypted_param,
                                           ypc::utc::crypto_prefix_arbitrary);
    if (ret) {
      LOG(ERROR) << "decrypt_message_with_prefix failed: "
                 << stbox::status_string(ret);
      return ret;
    }

  }

  virtual ~oram_sealed_data_provider() {}

  
  virtual bool process() {
    uint32_t b_id = get_block_id();
  }

  virtual data_source_output_t output_value() {
    data_source_output_t ret;
    ret.set<nt<bytes>::data>(m_result);
    return ret;
  }

private:
  uint32_t get_block_id() {
    uint32_t b_id;
    // 反序列化
    auto pkg = make_package<oram::input_buf_t>::from_bytes(m_decrypted_param);
    std::string m_param = pkg.get<oram::input_buf>();

    get_block_id_OCALL(m_param, &b_id);
    return b_id;
  }

protected:
  struct oram_header {
    uint32_t stash_size;
    uint32_t block_num;
    uint32_t bucket_num_N;
    uint8_t level_num_L;
    uint32_t bucket_str_size;
    uint32_t row_length;
    uint32_t batch_str_size;
  };

  bytes m_actual_data_hash;
  bytes m_private_key;
  bytes m_decrypted_param;

  oram_header m_header;
  std::vector<oram::block_t> m_stash;
  std::vector<uint32_t> m_position_map;
  // stbox::bytes m_encrypted_position_map;
  stbox::bytes m_encrypted_path;
  std::vector<oram::bucket_pkg_t> m_decrypted_path;
  stbox::bytes m_result;

};
} // namespace ypc