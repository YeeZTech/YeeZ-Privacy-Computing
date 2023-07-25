#pragma once
#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/corecommon/oram_types.h"
#include <sgx_trts.h>
#include "ypc/common/crypto_prefix.h"
#include "ypc/core_t/analyzer/oasm_lib.h"

#include "hpda/extractor/extractor_base.h"
#include "ypc/common/limits.h"
#include "ypc/core_t/analyzer/eparser_t_interface.h"
#include "ypc/core_t/ecommon/package.h"
#include "ypc/corecommon/package.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#include "ypc/stbox/tsgx/channel/dh_session_initiator.h"
#include "ypc/stbox/tsgx/log.h"
#include <ff/util/ntobject.h>


using oram_ntt = ypc::oram::nt<stbox::bytes>;

namespace ypc {
template <typename Crypto>
class oram_sealed_data_provider : public data_source_with_dhash {
  typedef Crypto crypto;
public:
  // TODO:这里需要获取请求参数param_data(加密)和param_private_key
  oram_sealed_data_provider(const stbox::bytes &data_hash,
                       const stbox::bytes &private_key,
                       const stbox::bytes &public_key,
                       const stbox::bytes &decrypted_param)
      : data_source_with_dhash(data_hash), m_private_key(private_key),
        m_public_key(public_key), m_decrypted_param(decrypted_param) {
    // magic string here, Do Not Change!
    crypto::hash_256(stbox::bytes("Fidelius"), m_actual_data_hash);
    m_data_reach_end = false;

    m_header.stash_size = oram::stash_size;

    // LOG(INFO) << "start access target batch";
    // 这里拿到包含目标batch
    bool ret = access();
    if (!ret) {
      LOG(ERROR) << "Failed to get target batch";
    }
  }

  virtual ~oram_sealed_data_provider() {}

  
  virtual bool process() {
    if(m_item_index < m_valid_item_num) {
      m_item_index++;
      return true;
    }
    if(m_item_index == m_valid_item_num) {
      m_data_reach_end = true;
      return false;
    }
    return false;
  }

  virtual data_source_output_t output_value() {
    data_source_output_t ret;
    // // LOG(INFO) << "output_value() m_item_index = " << m_item_index;
    ret.set<nt<bytes>::data>(m_items[m_item_index - 1]);
    return ret;
  }

  virtual const bytes &data_hash() const { return m_actual_data_hash; }
  const bytes &private_key() const { return m_private_key; }

private:

  bool access() {
    bool ret = download_oram_params();
    if (!ret) {
      LOG(ERROR) << "Failed to download_oram_params\n";
      return false;
    }
    // LOG(INFO) << "download_oram_params() suc!";
    // LOG(INFO) << "m_header.batch_str_size : " << m_header.batch_str_size;

    uint32_t block_id;
    ret = get_block_id(block_id);
    if (!ret) {
      LOG(ERROR) << "Failed to get_block_id\n";
      return false;
    }
    // LOG(INFO) << "get_block_id(b_id) suc!";
    // LOG(INFO) << "b_id = " << b_id;

    ret = download_position_map();
    if (!ret) {
      LOG(ERROR) << "Failed to download_position_map\n";
      return false;
    }
    // LOG(INFO) << "download_position_map() succ! ";

    unsigned char random_value[4];
    sgx_status_t se_ret = sgx_read_rand((unsigned char*) random_value, 4); 
    if (se_ret != SGX_SUCCESS) {
      LOG(ERROR) << "Failed to generate rand number\n";
      return false;
    }
    uint32_t new_leaf = *((uint32_t *)random_value) % (1 << m_header.level_num_L) + 1;

    // uint32_t leaf = m_position_map[b_id];
    // m_position_map[b_id] = new_leaf;
    uint32_t leaf;

    oarray_search(m_position_map, block_id, &leaf, new_leaf, m_position_map.size());

    // LOG(INFO) << "new_leaf: " << new_leaf;
    // LOG(INFO) << "leaf: " << leaf;

    ret = download_path(leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to download path\n";
      return false;
    }
    // LOG(INFO) << "download_path(leaf) succ! ";

    ret = download_stash();
    if (!ret) {
      LOG(ERROR) << "Failed to download stash\n";
      return false;
    }
    // LOG(INFO) << "download_stash() succ! ";

    ret = decrypt_path();
    if (!ret) {
      LOG(ERROR) << "Failed to decrypt path\n";
      return false;
    }
    // LOG(INFO) << "decrypt_path() succ! ";

    ret = access_in_stash(block_id, new_leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to access in stash\n";
      return false;
    }
    // LOG(INFO) << "access_in_stash(b_id, new_leaf) succ! ";

    rebuild_new_path(leaf);
    // LOG(INFO) << "rebuild_new_path(leaf) succ! ";

    ret = encrypt_path();
    if (!ret) {
      LOG(ERROR) << "Failed to encrypt path\n";
      return false;
    }
    // LOG(INFO) << "encrypt_path() succ! ";

    ret = update_position_map();
    if (!ret) {
      LOG(ERROR) << "Failed to update position map\n";
      return false;
    }
    // LOG(INFO) << "update_position_map() succ! ";

    ret = upload_path(leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to upload path\n";
      return false;
    }
    // LOG(INFO) << "upload_path(leaf) succ! ";

    ret = update_stash();
    if (!ret) {
      LOG(ERROR) << "Failed to update stash\n";
      return false;
    }
    // LOG(INFO) << "update_stash() succ! ";

    return true;
  }
  
  void oarray_search(std::vector<uint32_t>& array, uint32_t loc, uint32_t *leaf, 
                     uint32_t newLabel, uint32_t N_level) {
    for(uint32_t i=0; i < N_level; ++i) {
      omove(i, &(array[i]), loc, leaf, newLabel);
    }
    return;
  }

  bool download_oram_params() {
    auto ret = stbox::ocall_cast<uint32_t>(download_oram_params_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), &m_header.block_num, 
        &m_header.bucket_num_N, &m_header.level_num_L, 
        &m_header.bucket_str_size, &m_header.batch_str_size);
    if (ret != stbox::stx_status::success) {
      return false;
    }

    // LOG(INFO) << "m_header.block_num : " << m_header.block_num;
    // LOG(INFO) << "m_header.bucket_num_N : " << m_header.bucket_num_N;
    // LOG(INFO) << "m_header.level_num_L : " << m_header.level_num_L;

    for(uint32_t i = 0; i < m_header.stash_size; ++i) {
      oram_ntt::block_t s_block;
      s_block.set<oram_ntt::block_id, oram_ntt::leaf_label, 
                  oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
                (0, 0, 0, stbox::bytes(m_header.batch_str_size));
      m_stash.push_back(s_block);
    }

    for(uint8_t i = 0; i < m_header.level_num_L + 1; ++i) {
      std::vector<oram_ntt::block_t> block_array;
      for(uint8_t j = 0; j < oram::BucketSizeZ; ++j) {
        oram_ntt::block_t p_block;
        p_block.set<oram_ntt::block_id, oram_ntt::leaf_label, 
                    oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
                  (0, 0, 0, stbox::bytes(m_header.batch_str_size));
        block_array.push_back(p_block);
      }
      oram_ntt::bucket_pkg_t bucket_pkg;
      bucket_pkg.set<oram_ntt::bucket>(block_array);
      m_decrypted_path.push_back(bucket_pkg);
    }

    return true;
  }

  bool get_block_id(uint32_t &block_id) {
    uint32_t b_id;
    bytes param_hash;
    crypto::hash_256(m_decrypted_param, param_hash);

    auto ret = stbox::ocall_cast<uint32_t>(get_block_id_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), 
        &b_id, param_hash.data(), param_hash.size());
    if (ret != stbox::stx_status::success) {
      return false;
    }
    
    block_id = b_id;
    return true;
  }

  bool download_position_map() {
    uint8_t *posmap;
    uint32_t posmap_len;

    auto ret = stbox::ocall_cast<uint32_t>(download_position_map_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), &posmap, &posmap_len);
    if (ret != stbox::stx_status::success) {
      return false;
    }
    // LOG(INFO) << "posmap_len : " << posmap_len;
    stbox::bytes posmap_str(posmap_len);
    memcpy(posmap_str.data(), posmap, posmap_len);

    // LOG(INFO) << "posmap_str : " << posmap_str;
    // LOG(INFO) << "posmap_str.size() : " << posmap_str.size();

    stbox::bytes decrypted_position_map_bytes;
    
    uint32_t status = crypto::decrypt_message_with_prefix(
        m_private_key, posmap_str, decrypted_position_map_bytes, ypc::utc::crypto_prefix_arbitrary);
    if (ret) {
      LOG(ERROR) << "decrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return false;
    }

    // LOG(INFO) << "decrypted_position_map_bytes.size() : " << decrypted_position_map_bytes.size();
    // LOG(INFO) << "decrypted_position_map_bytes : " << decrypted_position_map_bytes;
    std::vector<uint32_t> position_map_array;
    try {
      oram_ntt::position_map_t position_map_pkg = make_package<oram_ntt::position_map_t>::from_bytes(decrypted_position_map_bytes);
      position_map_array = position_map_pkg.get<oram_ntt::position_map>();
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_package got: " << e.what();
      return false;
    }

    // LOG(INFO) << "position_map_array.size() : " << position_map_array.size();

    m_position_map.clear();
    for(const auto& element : position_map_array) {
      m_position_map.push_back(element);
    }

    return true;
  }

  bool update_position_map() {
    oram_ntt::position_map_t position_map_pkg;
    position_map_pkg.set<oram_ntt::position_map>(m_position_map);
    stbox::bytes position_map_bytes = make_bytes<stbox::bytes>::for_package(position_map_pkg);
    stbox::bytes encrypted_position_map_bytes;
    uint32_t status = crypto::encrypt_message_with_prefix(
      m_public_key, position_map_bytes, ypc::utc::crypto_prefix_arbitrary, encrypted_position_map_bytes);
    if (status) {
      LOG(ERROR) << "encrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return false;
    }

    // LOG(INFO) << "encrypted_position_map_bytes: " << encrypted_position_map_bytes;

    auto ret = stbox::ocall_cast<uint32_t>(update_position_map_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), 
        encrypted_position_map_bytes.data(), encrypted_position_map_bytes.size());
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "update_position_map_OCALL fail!";
      return false;
    }

    return true;
  }

  bool download_path(uint32_t leaf) {
    uint8_t *encrypted_path;
    uint32_t encrypted_path_len;

    auto ret = stbox::ocall_cast<uint32_t>(download_path_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), 
        leaf, &encrypted_path, &encrypted_path_len);
    if (ret != stbox::stx_status::success) {
      return false;
    }

    m_encrypted_path = stbox::bytes(encrypted_path_len);
    memcpy(m_encrypted_path.data(), encrypted_path, encrypted_path_len);

    return true;
  }

  bool download_stash() {
    uint8_t *stash;
    uint32_t stash_len;

    auto ret = stbox::ocall_cast<uint32_t>(download_stash_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), 
        &stash, &stash_len);
    if (ret != stbox::stx_status::success) {
      return false;
    }

    // LOG(INFO) << "stash_len : " << stash_len;

    if(stash_len > 0) {
      stbox::bytes stash_str(stash_len);
      memcpy(stash_str.data(), stash, stash_len);

      stbox::bytes decrypted_stash_bytes;
      uint32_t status = crypto::decrypt_message_with_prefix(
          m_private_key, stash_str, decrypted_stash_bytes, ypc::utc::crypto_prefix_arbitrary);
      if (status) {
        LOG(ERROR) << "decrypt_message_with_prefix fail: "
                    << stbox::status_string(status);
        return false;
      }

      auto stash_pkg = make_package<oram_ntt::bucket_pkg_t>::from_bytes(decrypted_stash_bytes);
      auto block_array = stash_pkg.get<oram_ntt::bucket>();

      // LOG(INFO) << "block_array.size() : " << block_array.size();
      uint32_t i = 0;
      for(const auto& element : block_array) {
        m_stash[i++] = element;
      }

      assert(m_header.stash_size == m_stash.size());
    }

    return true;
  }

  bool decrypt_path() {
    oram_ntt::path_pkg_t path_pkg = make_package<oram_ntt::path_pkg_t>::from_bytes(m_encrypted_path);
    auto bucket_array = path_pkg.get<oram_ntt::path>();
    for(const auto& encrypted_bucket_str : bucket_array) {
      stbox::bytes decrypted_bucket_str;
      uint32_t status = crypto::decrypt_message_with_prefix(
          m_private_key, encrypted_bucket_str, decrypted_bucket_str, ypc::utc::crypto_prefix_arbitrary);
      if (status) {
        LOG(ERROR) << "decrypt_message_with_prefix fail: "
                    << stbox::status_string(status);
        return false;
      }

      oram_ntt::bucket_pkg_t bucket_pkg = make_package<oram_ntt::bucket_pkg_t>::from_bytes(decrypted_bucket_str);
      auto block_array = bucket_pkg.get<oram_ntt::bucket>();

      for(oram_ntt::block_t e_block : block_array) {
        for(uint32_t k = 0; k < m_stash.size(); ++k) {
          if(e_block.get<oram_ntt::block_id>() > 0 && m_stash[k].get<oram_ntt::block_id>() == 0) {
            m_stash[k].set<oram_ntt::block_id, oram_ntt::leaf_label, oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
              (e_block.get<oram_ntt::block_id>(), e_block.get<oram_ntt::leaf_label>(), 
               e_block.get<oram_ntt::valid_item_num>(), e_block.get<oram_ntt::encrypted_batch>());
            break;
          }
        }
      }

    }

    return true;
  }

  bool access_in_stash(uint32_t block_id, uint32_t new_leaf) {
    for(uint32_t i = 0; i < m_stash.size(); ++i) {
      if(m_stash[i].get<oram_ntt::block_id>() == block_id) {
        m_valid_item_num = m_stash[i].get<oram_ntt::valid_item_num>();
        if(m_valid_item_num == 0) {
          LOG(ERROR) << "fail, m_valid_item_num == 0 ";
          return false;
        }

        stbox::bytes encrypted_batch_str = m_stash[i].get<oram_ntt::encrypted_batch>();
        stbox::bytes decrypted_batch_str;
        uint32_t status = crypto::decrypt_message_with_prefix(
            m_private_key, encrypted_batch_str, decrypted_batch_str, ypc::utc::crypto_prefix_arbitrary);
        if (status) {
          LOG(ERROR) << "decrypt_message_with_prefix fail: "
                      << stbox::status_string(status);
          return false;
        }

        typedef nt<stbox::bytes> ntt;
        try {
          auto pkg = make_package<ntt::batch_data_pkg_t>::from_bytes(decrypted_batch_str);
          m_items = pkg.get<ntt::batch_data>();
          if (m_items.size() == 0) {
            LOG(ERROR) << "fail, m_items.size() == 0 ";
            return false;
          }

          // TODO:m_actual_data_hash

          m_item_index = 0;
          m_stash[i].set<oram_ntt::leaf_label>(new_leaf);
          return true;
        } catch (const std::exception &e) {
          LOG(ERROR) << "make_package got: " << e.what();
          return false;
        }

      }

    }

    return false;
  }

  uint8_t get_level(uint32_t leaf1, uint32_t leaf2) {
    uint32_t leaf1_index = leaf1 - 1 + (1 << m_header.level_num_L) - 1;
    uint32_t leaf2_index = leaf2 - 1 + (1 << m_header.level_num_L) - 1;

    while(leaf1_index != leaf2_index) {
        leaf1_index = (leaf1_index - 1) / 2;
        leaf2_index = (leaf2_index - 1) / 2;
    }

    return floor(log2(leaf1_index + 1));
  }

  void rebuild_new_path(uint32_t leaf) {
    m_decrypted_path.clear();
    for(uint8_t i = 0; i < m_header.level_num_L + 1; ++i) {
      std::vector<oram_ntt::block_t> block_array;
      for(uint8_t j = 0; j < oram::BucketSizeZ; ++j) {
        oram_ntt::block_t p_block;
        p_block.set<oram_ntt::block_id, oram_ntt::leaf_label, 
            oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
            (0, 0, 0, stbox::bytes(m_header.batch_str_size));
        block_array.push_back(p_block);
      }

      oram_ntt::bucket_pkg_t bucket_pkg;
      bucket_pkg.set<oram_ntt::bucket>(block_array);
      m_decrypted_path.push_back(bucket_pkg);
    }

    for(uint32_t i = 0; i < m_stash.size(); ++i) {
      if(m_stash[i].get<oram_ntt::block_id>() > 0) {
        uint8_t low_level = get_level(leaf, m_stash[i].get<oram_ntt::leaf_label>());
        for(int level = low_level; level >= 0; --level) {
          for(uint8_t k = 0; k < oram::BucketSizeZ; ++k) {
            if(m_decrypted_path[level].get<oram_ntt::bucket>()[k].get<oram_ntt::block_id>() == 0) {
              m_decrypted_path[level].get<oram_ntt::bucket>()[k].set
                  <oram_ntt::block_id, oram_ntt::leaf_label, 
                  oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
                  (m_stash[i].get<oram_ntt::block_id>(), m_stash[i].get<oram_ntt::leaf_label>(), 
                  m_stash[i].get<oram_ntt::valid_item_num>(), m_stash[i].get<oram_ntt::encrypted_batch>());
              m_stash[i].set<oram_ntt::block_id, oram_ntt::leaf_label, 
                  oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
                  (0, 0, 0, stbox::bytes(m_header.batch_str_size));
              break;
            }
          }

          if(m_stash[i].get<oram_ntt::block_id>() == 0) {
            break;
          }
        }
      }
    }
  }

  bool update_stash() {
    std::vector<oram_ntt::block_t> stash_block_array;
    for(uint32_t i = 0; i < m_stash.size(); ++i) {
      if (m_stash[i].get<oram_ntt::block_id>() > 0) {
        stash_block_array.push_back(m_stash[i]);
      }
    }

    // LOG(INFO) << "stash_block_array.size(): " << stash_block_array.size();

    oram_ntt::bucket_pkg_t stash_pkg;
    stash_pkg.set<oram_ntt::bucket>(stash_block_array);
    stbox::bytes stash_str = make_bytes<stbox::bytes>::for_package(stash_pkg);
    stbox::bytes encrypted_stash_str;
    uint32_t status = crypto::encrypt_message_with_prefix(
      m_public_key, stash_str, ypc::utc::crypto_prefix_arbitrary, encrypted_stash_str);
    if (status) {
      LOG(ERROR) << "encrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return false;
    }

    // LOG(INFO) << "encrypted_stash_str.size() : " << encrypted_stash_str.size();

    auto ret = stbox::ocall_cast<uint32_t>(update_stash_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), 
        encrypted_stash_str.data(), encrypted_stash_str.size());
    
    if (ret != stbox::stx_status::success) {
      return false;
    }

    return true;
  }

  bool encrypt_path() {
    std::vector<stbox::bytes> encrypted_path_array;
    for(oram_ntt::bucket_pkg_t bucket_pkg : m_decrypted_path) {
      stbox::bytes bucket_str = make_bytes<stbox::bytes>::for_package(bucket_pkg);
      stbox::bytes encrypted_bucket_bytes;
      uint32_t status = crypto::encrypt_message_with_prefix(
        m_public_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
      if (status) {
        LOG(ERROR) << "encrypt_message_with_prefix fail: "
                    << stbox::status_string(status);
        return false;
      }

      // LOG(INFO) << "encrypted_bucket_bytes : " << encrypted_bucket_bytes;
      encrypted_path_array.push_back(encrypted_bucket_bytes);
    }

    oram_ntt::path_pkg_t path_pkg;
    path_pkg.set<oram_ntt::path>(encrypted_path_array);
    m_encrypted_path = make_bytes<stbox::bytes>::for_package(path_pkg);

    return true;
  }

  bool upload_path(uint32_t leaf) {
    auto ret = stbox::ocall_cast<uint32_t>(upload_path_OCALL)
        (m_expect_data_hash.data(), m_expect_data_hash.size(), 
        leaf, m_encrypted_path.data(), m_encrypted_path.size());
    if (ret != stbox::stx_status::success) {
      return false;
    }

    return true;
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
  std::vector<stbox::bytes> m_items;
  size_t m_item_index;
  bytes m_private_key;
  bytes m_public_key;
  bytes m_decrypted_param;

  oram_header m_header;
  std::vector<oram_ntt::block_t> m_stash;
  std::vector<uint32_t> m_position_map;
  stbox::bytes m_encrypted_path;
  std::vector<oram_ntt::bucket_pkg_t> m_decrypted_path;
  uint32_t m_valid_item_num;
  

};
} // namespace ypc