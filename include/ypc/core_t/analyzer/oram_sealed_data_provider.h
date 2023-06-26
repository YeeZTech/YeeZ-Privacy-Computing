#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/corecommon/ttypes.h"

namespace ypc {
template <typename Crypto>
class oram_sealed_data_provider : public data_source_with_dhash {
  typedef Crypto crypto;
public:
  // TODO:这里需要获取请求参数
  oram_sealed_data_provider(const stbox::bytes &data_hash,
                       const stbox::bytes &private_key)
      : data_source_with_dhash(data_hash), m_private_key(private_key) {
    // magic string here, Do Not Change!
    crypto::hash_256(stbox::bytes("Fidelius"), m_actual_data_hash);
    // TODO:并不需要m_data_reach_end
    m_data_reach_end = false;
  }

  virtual ~oram_sealed_data_provider() {}

  
  virtual bool process() {
    
  }

  virtual data_source_output_t output_value() {
    data_source_output_t ret;
    ret.set<nt<bytes>::data>(m_result);
    return ret;
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
  oram_header m_header;
  std::vector<oram::block_t> m_stash;
  std::vector<uint32_t> m_position_map;
  // stbox::bytes m_encrypted_position_map;
  stbox::bytes m_encrypted_path;
  std::vector<oram::bucket_pkg_t> m_decrypted_path;
  stbox::bytes m_result;

};
} // namespace ypc