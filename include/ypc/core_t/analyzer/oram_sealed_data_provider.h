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
typedef ypc::nt<stbox::bytes> ntt;

namespace ypc {
template <typename Crypto>
// class oram_sealed_data_provider : public data_source_with_dhash {
class oram_sealed_data_provider : public data_source_with_merkle_hash {
  typedef Crypto crypto;
public:
  oram_sealed_data_provider(const stbox::bytes &data_hash,
                       const stbox::bytes &private_key,
                       const stbox::bytes &public_key,
                       const stbox::bytes &decrypted_param)
      : data_source_with_merkle_hash(data_hash), m_private_key(private_key),
        m_public_key(public_key), m_decrypted_param(decrypted_param) {
    // magic string here, Do Not Change!
    m_header.stash_size = oram::stash_size;
    
  }

  virtual ~oram_sealed_data_provider() {}

  
  virtual bool process() {
    if(! m_is_access_executed) {
      bool ret = access();
      if (!ret) {
        LOG(ERROR) << "Failed to get target batch";
      }

      m_is_access_executed = true;
      return true;
    }
    if(m_item_index + 1 < m_valid_item_num) {
      m_item_index++;
      return true;
    }

    return false;
  }

  virtual data_source_output_t output_value() {
    data_source_output_t ret;
    ret.set<nt<bytes>::data>(m_items[m_item_index]);
    return ret;
  }

  virtual const std::vector<bytes> &data_hash() const { return m_actual_data_hash; }
  const bytes &private_key() const { return m_private_key; }

private:

  bool access() {
    bool ret = download_oram_params();
    if (!ret) {
      LOG(ERROR) << "Failed to download_oram_params\n";
      return false;
    }
    
    uint32_t block_id;
    ret = get_block_id(block_id);
    if (!ret) {
      LOG(ERROR) << "Failed to get block_id\n";
      return false;
    }

    ret = download_position_map();
    if (!ret) {
      LOG(ERROR) << "Failed to download position map\n";
      return false;
    }

    unsigned char random_value[4];
    sgx_status_t se_ret = sgx_read_rand((unsigned char*) random_value, 4); 
    if (se_ret != SGX_SUCCESS) {
      LOG(ERROR) << "Failed to generate rand number\n";
      return false;
    }
    uint32_t new_leaf = *((uint32_t *)random_value) % (1 << m_header.level_num_L) + 1;
    uint32_t leaf;

    oarray_search(m_position_map, block_id, &leaf, new_leaf, m_position_map.size());

    ret = download_merkle_hash(leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to download merkle hash\n";
      return false;
    }

    ret = download_path(leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to download path\n";
      return false;
    }

    ret = download_stash();
    if (!ret) {
      LOG(ERROR) << "Failed to download stash\n";
      return false;
    }

    ret = decrypt_path();
    if (!ret) {
      LOG(ERROR) << "Failed to decrypt path\n";
      return false;
    }

    ret = access_in_stash(block_id, new_leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to access in stash\n";
      return false;
    }

    rebuild_new_path(leaf);

    ret = recalculate_hash();
    if (!ret) {
      LOG(ERROR) << "Failed to recalculate hash\n";
      return false;
    }

    ret = encrypt_path();
    if (!ret) {
      LOG(ERROR) << "Failed to encrypt path\n";
      return false;
    }

    ret = update_position_map();
    if (!ret) {
      LOG(ERROR) << "Failed to update position map\n";
      return false;
    }

    ret = upload_path(leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to upload path\n";
      return false;
    }

    ret = update_merkle_hash(leaf);
    if (!ret) {
      LOG(ERROR) << "Failed to update merkle hash\n";
      return false;
    }

    ret = update_stash();
    if (!ret) {
      LOG(ERROR) << "Failed to update stash\n";
      return false;
    }

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
        (m_expect_root_hash.data(), m_expect_root_hash.size(), &m_header.block_num, 
        &m_header.bucket_num_N, &m_header.level_num_L, 
        &m_header.bucket_str_size, &m_header.batch_str_size);
    if (ret != stbox::stx_status::success) {
      return false;
    }

    for(uint32_t i = 0; i < m_header.stash_size; ++i) {
      oram_ntt::block_t s_block;
      s_block.set<oram_ntt::block_id, oram_ntt::leaf_label, 
                  oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
                (0, 0, 0, stbox::bytes(m_header.batch_str_size));
      m_stash.push_back(s_block);
    }

    return true;
  }

  bool get_block_id(uint32_t &block_id) {
    uint32_t b_id;
    bytes param_hash;
    crypto::hash_256(m_decrypted_param, param_hash);

    auto ret = stbox::ocall_cast<uint32_t>(get_block_id_OCALL)
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
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
        (m_expect_root_hash.data(), m_expect_root_hash.size(), &posmap, &posmap_len);
    if (ret != stbox::stx_status::success) {
      return false;
    }
    
    stbox::bytes posmap_str(posmap_len);
    memcpy(posmap_str.data(), posmap, posmap_len);

    stbox::bytes decrypted_position_map_bytes;
    uint32_t status = crypto::decrypt_message_with_prefix(
        m_private_key, posmap_str, decrypted_position_map_bytes, ypc::utc::crypto_prefix_arbitrary);
    if (ret) {
      LOG(ERROR) << "decrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return false;
    }

    try {
      oram_ntt::position_map_t position_map_pkg = make_package<oram_ntt::position_map_t>::from_bytes(decrypted_position_map_bytes);
      m_position_map = position_map_pkg.get<oram_ntt::position_map>();
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_package got: " << e.what();
      return false;
    }

    return true;
  }

  bool update_position_map() {
    oram_ntt::position_map_t position_map_pkg;
    position_map_pkg.set<oram_ntt::position_map>(m_position_map);
    stbox::bytes position_map_bytes;
    try {
      position_map_bytes = make_bytes<stbox::bytes>::for_package(position_map_pkg);
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_bytes got: " << e.what();
      return false;
    }
    stbox::bytes encrypted_position_map_bytes;
    uint32_t status = crypto::encrypt_message_with_prefix(
      m_public_key, position_map_bytes, ypc::utc::crypto_prefix_arbitrary, encrypted_position_map_bytes);
    if (status) {
      LOG(ERROR) << "encrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return false;
    }

    auto ret = stbox::ocall_cast<uint32_t>(update_position_map_OCALL)
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
        encrypted_position_map_bytes.data(), encrypted_position_map_bytes.size());
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "update_position_map_OCALL fail!";
      return false;
    }

    return true;
  }

  bool download_merkle_hash(uint32_t leaf) {
    uint8_t *merkle_hash;
    uint32_t merkle_hash_len;

    auto ret = stbox::ocall_cast<uint32_t>(download_merkle_hash_OCALL)
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
        leaf, &merkle_hash, &merkle_hash_len);
    if (ret != stbox::stx_status::success) {
      return false;
    }

    bytes merkle_hash_str(merkle_hash_len);
    memcpy(merkle_hash_str.data(), merkle_hash, merkle_hash_len);

    try {
      oram_ntt::merkle_hash_pkg_t merkle_hash_pkg = 
        make_package<oram_ntt::merkle_hash_pkg_t>::from_bytes(merkle_hash_str);
      m_merkle_hash_array = merkle_hash_pkg.get<oram_ntt::merkle_hash>();
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_package got: " << e.what();
      return false;
    }

    for(const auto& hash_p : m_merkle_hash_array) {
      if(hash_p.get<oram_ntt::in_path>()) {
        m_expect_data_hash.push_back(hash_p.get<oram_ntt::data_hash>());
      }
    }

    return true;
  }

  bool download_path(uint32_t leaf) {
    uint8_t *encrypted_path;
    uint32_t encrypted_path_len;

    auto ret = stbox::ocall_cast<uint32_t>(download_path_OCALL)
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
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
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
        &stash, &stash_len);
    if (ret != stbox::stx_status::success) {
      return false;
    }

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

      try {
        auto stash_pkg = make_package<oram_ntt::bucket_pkg_t>::from_bytes(decrypted_stash_bytes);
        auto block_array = stash_pkg.get<oram_ntt::bucket>();
        
        uint32_t i = 0;
        for(const auto& element : block_array) {
          m_stash[i++] = element;
        }

      } catch (const std::exception &e) {
        LOG(ERROR) << "make_package got: " << e.what();
        return false;
      }

    }

    return true;
  }

  bool decrypt_path() {
    std::vector<stbox::bytes> bucket_array;

    try {
      oram_ntt::path_pkg_t path_pkg = make_package<oram_ntt::path_pkg_t>::from_bytes(m_encrypted_path);
      bucket_array = path_pkg.get<oram_ntt::path>();
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_package got: " << e.what();
      return false;
    }

    // calculate m_actual_data_hash
    for(const auto& encrypted_bucket_str : bucket_array) {
      stbox::bytes decrypted_bucket_str;
      uint32_t status = crypto::decrypt_message_with_prefix(
          m_private_key, encrypted_bucket_str, decrypted_bucket_str, ypc::utc::crypto_prefix_arbitrary);
      if (status) {
        LOG(ERROR) << "decrypt_message_with_prefix fail: "
                    << stbox::status_string(status);
        return false;
      }

      stbox::bytes data_hash;
      crypto::hash_256(bytes("Fidelius"), data_hash);
      
      std::vector<oram_ntt::block_t> block_array;
      try {
        oram_ntt::bucket_pkg_t bucket_pkg = make_package<oram_ntt::bucket_pkg_t>::from_bytes(decrypted_bucket_str);
        m_decrypted_path.push_back(bucket_pkg);
        block_array = bucket_pkg.get<oram_ntt::bucket>();
      } catch (const std::exception &e) {
        LOG(ERROR) << "make_package got: " << e.what();
        return false;
      }

      for(oram_ntt::block_t e_block : block_array) {
        for(uint32_t k = 0; k < m_stash.size(); ++k) {
          if(e_block.get<oram_ntt::block_id>() > 0 && m_stash[k].get<oram_ntt::block_id>() == 0) {
            m_stash[k].set<oram_ntt::block_id, oram_ntt::leaf_label, oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
              (e_block.get<oram_ntt::block_id>(), e_block.get<oram_ntt::leaf_label>(), 
               e_block.get<oram_ntt::valid_item_num>(), e_block.get<oram_ntt::encrypted_batch>());
            break;
          }
        }

        stbox::bytes encrypted_batch = e_block.get<oram_ntt::encrypted_batch>();
        stbox::bytes decrypted_batch_str;
        status = crypto::decrypt_message_with_prefix(
            m_private_key, encrypted_batch, decrypted_batch_str, ypc::utc::crypto_prefix_arbitrary);
        if (status) {
          LOG(ERROR) << "decrypt_message_with_prefix fail: "
                      << stbox::status_string(status);
          return false;
        }

        try {
          auto pkg = make_package<ntt::batch_data_pkg_t>::from_bytes(decrypted_batch_str);
          auto items = pkg.get<ntt::batch_data>();

          for (auto b : items) {
            stbox::bytes k_hash = data_hash + b;
            crypto::hash_256(k_hash, data_hash);
          }

        } catch (const std::exception &e) {
          LOG(ERROR) << "make_package got: " << e.what();
          return false;
        }
      }

      m_actual_data_hash.push_back(data_hash);


    }

    for(int i = m_actual_data_hash.size() - 2; i >= 0; --i) {
      stbox::bytes k_hash = m_actual_data_hash[i] + m_merkle_hash_array[2*i + 1].get<oram_ntt::data_hash>();
      crypto::hash_256(k_hash, m_actual_data_hash[i]);

      k_hash = m_actual_data_hash[i] + m_merkle_hash_array[2*i + 2].get<oram_ntt::data_hash>();
      crypto::hash_256(k_hash, m_actual_data_hash[i]);
    }

    return true;
  }

  bool access_in_stash(uint32_t block_id, uint32_t new_leaf) {
    for(uint32_t i = 0; i < m_stash.size(); ++i) {
      if(m_stash[i].get<oram_ntt::block_id>() == block_id) {
        m_valid_item_num = m_stash[i].get<oram_ntt::valid_item_num>();
        if(m_valid_item_num == 0) {
          LOG(ERROR) << "fail, the value of valid item num is 0 ";
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

        try {
          auto pkg = make_package<ntt::batch_data_pkg_t>::from_bytes(decrypted_batch_str);
          m_items = pkg.get<ntt::batch_data>();
          if (m_items.size() == 0) {
            LOG(ERROR) << "fail, m_items.size() == 0 ";
            return false;
          }

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
    for(auto &bu : m_decrypted_path) {
      auto block_array = bu.get<oram_ntt::bucket>();
      for(uint8_t j = 0; j < oram::BucketSizeZ; ++j) {
        block_array[j].set<oram_ntt::block_id, oram_ntt::leaf_label, oram_ntt::valid_item_num>(0, 0, 0);
      }
      bu.set<oram_ntt::bucket>(block_array);
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
                  oram_ntt::valid_item_num>(0, 0, 0);
              
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


    oram_ntt::bucket_pkg_t stash_pkg;
    stash_pkg.set<oram_ntt::bucket>(stash_block_array);
    stbox::bytes stash_str;
    try {
      stash_str = make_bytes<stbox::bytes>::for_package(stash_pkg);
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_bytes got: " << e.what();
      return false;
    }
    stbox::bytes encrypted_stash_str;
    uint32_t status = crypto::encrypt_message_with_prefix(
      m_public_key, stash_str, ypc::utc::crypto_prefix_arbitrary, encrypted_stash_str);
    if (status) {
      LOG(ERROR) << "encrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return false;
    }

    auto ret = stbox::ocall_cast<uint32_t>(update_stash_OCALL)
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
        encrypted_stash_str.data(), encrypted_stash_str.size());
    
    if (ret != stbox::stx_status::success) {
      return false;
    }

    return true;
  }

  bool recalculate_hash() {
    uint32_t index = 0;
    for(auto &bu : m_decrypted_path) {
      
      stbox::bytes data_hash;
      crypto::hash_256(bytes("Fidelius"), data_hash);

      auto block_array = bu.get<oram_ntt::bucket>();
      for(auto &e_block : block_array) {
        stbox::bytes encrypted_batch = e_block.get<oram_ntt::encrypted_batch>();
        stbox::bytes decrypted_batch_str;
        uint32_t status = crypto::decrypt_message_with_prefix(
            m_private_key, encrypted_batch, decrypted_batch_str, ypc::utc::crypto_prefix_arbitrary);
        if (status) {
          LOG(ERROR) << "decrypt_message_with_prefix fail: "
                      << stbox::status_string(status);
          return false;
        }

        try {
          auto pkg = make_package<ntt::batch_data_pkg_t>::from_bytes(decrypted_batch_str);
          auto items = pkg.get<ntt::batch_data>();

          for (auto b : items) {
            stbox::bytes k_hash = data_hash + b;
            crypto::hash_256(k_hash, data_hash);
          }
        } catch (const std::exception &e) {
          LOG(ERROR) << "make_package got: " << e.what();
          return false;
        }

        // TODO:虚块重新填充
        // for(uint32_t j = 0; j < items.size(); ++j) {
        //   std::string dummy_item_str;
        //   generate_random_str(dummy_item_str, item_size);
        //   bytes dummy_item(dummy_item_str);
        //   items[i] = dummy_item;
        //   stbox::bytes k_hash = data_hash + dummy_item;
        //   crypto::hash_256(k_hash, data_hash);
        // }

        // 重新加密放入m_decrypted_path中

      }

      while(!m_merkle_hash_array[index].get<oram_ntt::in_path>()) {
        ++index;
      }
      m_merkle_hash_array[index++].set<oram_ntt::data_hash>(data_hash);
      
    }

    for(int i = m_header.level_num_L - 1; i >= 0; --i) {

      if(i != 0 && m_merkle_hash_array[2*i - 1].get<oram_ntt::in_path>()) {
        stbox::bytes k_hash = m_merkle_hash_array[2*i - 1].get<oram_ntt::data_hash>() 
                              + m_merkle_hash_array[2*i + 1].get<oram_ntt::data_hash>();
        stbox::bytes data_hash;
        crypto::hash_256(k_hash, data_hash);

        k_hash = data_hash + m_merkle_hash_array[2*i + 2].get<oram_ntt::data_hash>();
        crypto::hash_256(k_hash, data_hash);

        m_merkle_hash_array[2*i - 1].set<oram_ntt::data_hash>(data_hash);
      } 

      if(m_merkle_hash_array[2*i].get<oram_ntt::in_path>()) {
        stbox::bytes k_hash = m_merkle_hash_array[2*i].get<oram_ntt::data_hash>() 
                              + m_merkle_hash_array[2*i + 1].get<oram_ntt::data_hash>();
        stbox::bytes data_hash;
        crypto::hash_256(k_hash, data_hash);

        k_hash = data_hash + m_merkle_hash_array[2*i + 2].get<oram_ntt::data_hash>();
        crypto::hash_256(k_hash, data_hash);

        m_merkle_hash_array[2*i].set<oram_ntt::data_hash>(data_hash);
      } 
    }

    return true;
  }

  bool encrypt_path() {
    std::vector<stbox::bytes> encrypted_path_array;
    for(oram_ntt::bucket_pkg_t bucket_pkg : m_decrypted_path) {
      stbox::bytes bucket_str;
      try {
        bucket_str = make_bytes<stbox::bytes>::for_package(bucket_pkg);
      } catch (const std::exception &e) {
        LOG(ERROR) << "make_bytes got: " << e.what();
        return false;
      }
      stbox::bytes encrypted_bucket_bytes;
      uint32_t status = crypto::encrypt_message_with_prefix(
        m_public_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
      if (status) {
        LOG(ERROR) << "encrypt_message_with_prefix fail: "
                    << stbox::status_string(status);
        return false;
      }

      encrypted_path_array.push_back(encrypted_bucket_bytes);
    }

    oram_ntt::path_pkg_t path_pkg;
    path_pkg.set<oram_ntt::path>(encrypted_path_array);
    try {
      m_encrypted_path = make_bytes<stbox::bytes>::for_package(path_pkg);
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_bytes got: " << e.what();
      return false;
    }

    return true;
  }

  bool upload_path(uint32_t leaf) {
    auto ret = stbox::ocall_cast<uint32_t>(upload_path_OCALL)
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
        leaf, m_encrypted_path.data(), m_encrypted_path.size());
    if (ret != stbox::stx_status::success) {
      return false;
    }

    return true;
  }

  bool update_merkle_hash(uint32_t leaf) {
    oram_ntt::merkle_hash_pkg_t merkle_hash_pkg;
    merkle_hash_pkg.set<oram_ntt::merkle_hash>(m_merkle_hash_array);
    bytes merkle_hash_str;
    try {
      merkle_hash_str = make_bytes<bytes>::for_package(merkle_hash_pkg);
    } catch (const std::exception &e) {
      LOG(ERROR) << "make_bytes got: " << e.what();
      return false;
    }

    auto ret = stbox::ocall_cast<uint32_t>(update_merkle_hash_OCALL)
        (m_expect_root_hash.data(), m_expect_root_hash.size(), 
        leaf, merkle_hash_str.data(), merkle_hash_str.size());
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

  std::vector<stbox::bytes> m_actual_data_hash;
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
  std::vector<oram_ntt::hash_pair> m_merkle_hash_array;
  bool m_is_access_executed;
  

};
} // namespace ypc