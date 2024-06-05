#include "ypc/corecommon/package.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#ifdef EXAMPLE_FM_NORMAL
#include <glog/logging.h>
typedef ypc::bytes bytes;
#else
#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/stbox/tsgx/log.h"
typedef stbox::bytes bytes;
#endif
#include "user_type.h"
#include "ypc/corecommon/data_source.h"
#include "ypc/corecommon/to_type.h"
#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/query/filter.h>
#include <string.h>


#include "ypc/core_t/analyzer/eparser_t_interface.h"
#include "ypc/common/crypto_prefix.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/oram_types.h"
#include <random>
#include <hpda/processor/transform/split.h>


using oram_ntt = ypc::oram::nt<bytes>;
using ntt = ypc::nt<bytes>;





define_nt(input_buf, std::string);
typedef ff::net::ntpackage<0, input_buf> input_buf_t;


class crypto_base {
public:
  virtual uint32_t encrypt_message_with_prefix(const bytes &public_key,
                                               const bytes &data,
                                               uint32_t prefix,
                                               bytes &cipher) = 0;
  virtual uint32_t hash_256(const bytes &msg, bytes &hash) = 0;
};
using crypto_ptr_t = std::shared_ptr<crypto_base>;
template <typename Crypto> class crypto_tool : public crypto_base {
public:
  using crypto_t = Crypto;
  virtual uint32_t encrypt_message_with_prefix(const bytes &public_key,
                                               const bytes &data,
                                               uint32_t prefix,
                                               bytes &cipher) {
    return crypto_t::encrypt_message_with_prefix(public_key, data, prefix,
                                                 cipher);
  }
  virtual uint32_t hash_256(const bytes &msg, bytes &hash) {
    return crypto_t::hash_256(msg, hash);
  }
};


bytes random_string(size_t len) {
  std::string ret(len, '0');
  static std::default_random_engine generator;
  static std::uniform_int_distribution<int> distribution(int('a'), int('z'));
  static auto rand = std::bind(distribution, generator);

  for (size_t i = 0; i < len; i++) {
    ret[i] = rand();
  }
  return bytes(ret.data(), ret.size());
}

bool push_dummy_block(std::vector<oram_ntt::block_t>& bucket_array, bytes &data_hash,
                      uint8_t count, uint64_t item_num_each_batch, uint64_t item_size,
                      const crypto_ptr_t &crypto_ptr, const bytes &public_key) {
  for(uint8_t i = 0; i < count; ++i) {
    oram_ntt::block_t b_block;

    std::vector<bytes> dummy_batch;
    for(uint32_t j = 0; j < item_num_each_batch; ++j) {
      bytes dummy_item = random_string(item_size);
      dummy_batch.push_back(dummy_item);
      bytes k_hash = data_hash + dummy_item;
      crypto_ptr->hash_256(k_hash, data_hash);
    }

    bytes encrypted_dummy_batch;
    bytes dummy_batch_str =
      ypc::make_bytes<bytes>::for_package<ntt::batch_data_pkg_t,
                                               ntt::batch_data>(dummy_batch);

    // encrypt dummy batch
    uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, dummy_batch_str, ypc::utc::crypto_prefix_arbitrary, encrypted_dummy_batch);
    if (status) {
      LOG(ERROR) << "encrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return false;
    }

    b_block.set<oram_ntt::block_id, oram_ntt::leaf_label, oram_ntt::valid_item_num, oram_ntt::encrypted_batch>(0, 0, 0, encrypted_dummy_batch);
    bucket_array.push_back(b_block);
  }
  return true;
}

uint32_t get_leaf_label(uint32_t bucket_index, uint8_t level_num_L) {
  // leftmost leaf node
  uint32_t leftmost_leaf_index = (1 << level_num_L) - 1;
  if(bucket_index >= leftmost_leaf_index) {
      return bucket_index - leftmost_leaf_index + 1;
  }

  // randomly select a path to the leaf node
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);

  if(dis(gen) == 0) {
      return get_leaf_label(2 * bucket_index + 1, level_num_L);
  }
  return get_leaf_label(2 * bucket_index + 2, level_num_L);
}

bool push_real_block(std::vector<oram_ntt::block_t>& bucket_array, bytes &data_hash,
                      uint32_t& block_id_value, uint32_t bucket_index, 
                      std::vector<uint32_t> &position_map_array, uint8_t level_num_L,
                      std::vector<bytes> &batch, uint32_t &batch_str_size,
                      uint64_t item_num_each_batch, uint64_t item_size, 
                      const crypto_ptr_t &crypto_ptr, const bytes &public_key) {
  oram_ntt::block_t b_block;
  uint32_t valid_item_num = batch.size();
  for(uint32_t i = 0; i < item_num_each_batch - valid_item_num; ++i) {
    ypc::bytes item = random_string(item_size);
    batch.push_back(item);
  }

  for(auto &item : batch) {
    ypc::bytes k_hash = data_hash + item;
    crypto_ptr->hash_256(k_hash, data_hash);
  }

  bytes encrypted_batch;
  ypc::bytes batch_str =
    ypc::make_bytes<ypc::bytes>::for_package<ntt::batch_data_pkg_t,
                                              ntt::batch_data>(batch);
  // encrypt batch
  uint32_t status = crypto_ptr->encrypt_message_with_prefix(
    public_key, batch_str, ypc::utc::crypto_prefix_arbitrary, encrypted_batch);
  if (status) {
    LOG(ERROR) << "encrypt_message_with_prefix fail: "
                << stbox::status_string(status);
    return false;
  }

  if(batch_str_size != encrypted_batch.size()) {
    batch_str_size = encrypted_batch.size();
  }

  uint32_t b_leaf_label = get_leaf_label(bucket_index, level_num_L);
  position_map_array[block_id_value] = b_leaf_label;
  b_block.set<oram_ntt::block_id, oram_ntt::leaf_label, 
      oram_ntt::valid_item_num, oram_ntt::encrypted_batch>
      (block_id_value++, b_leaf_label, valid_item_num, encrypted_batch);
  bucket_array.push_back(b_block);

  return true;
}


class convert_parser2 {
public:
  convert_parser2() {}
  convert_parser2(ypc::data_source<bytes> *source) : m_source(source){};

  inline bytes do_parse(const bytes &param) {
    bytes result;
    LOG(INFO) << "do convert_parse2";
    ypc::to_type<bytes, user_item_t> converter(m_source);

    crypto_ptr_t crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
    bytes pub_key(param);


    // 1. read header
    ypc::oram::header osf_header{};

    auto ret = stbox::ocall_cast<uint32_t>(download_convert_params_ocall)
      (&osf_header.block_num, &osf_header.oram_tree_filepos, &osf_header.item_num_each_batch, &osf_header.item_size);
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "download_convert_params_ocall fail!";
      return result;
    }
    
    uint32_t real_bucket_num = ceil(static_cast<double>(osf_header.block_num) / ypc::oram::BucketSizeZ);
    osf_header.level_num_L = ceil(log2(real_bucket_num + 1)) - 1; 
    osf_header.bucket_num_N = (1 << (osf_header.level_num_L + 1)) - 1;
    osf_header.id_map_filepos = sizeof(osf_header);


    // 2. write ORAM tree
    LOG(INFO) << "write ORAM tree";
    std::vector<bytes> data_hash_array;

    // from which bucket to start writing real blocks
    uint8_t lastbucket_realblocknum = osf_header.block_num % ypc::oram::BucketSizeZ;
    uint32_t bucket_index = 0; // bucket index in ORAM tree
    uint32_t block_id_value = 1; // block_id_value <= osf_header.block_num


    // 2.1 write buckets full of dummy blocks
    LOG(INFO) << "write buckets full of dummy blocks";  
    // osf.seekp(osf_header.oram_tree_filepos, osf.beg);
    int64_t filepos = osf_header.oram_tree_filepos;
    for(uint32_t i = 0; i < osf_header.bucket_num_N - real_bucket_num; ++i) {
      std::vector<oram_ntt::block_t> bucket_array;
      bytes data_hash;
      crypto_ptr->hash_256(bytes("Fidelius"), data_hash);
      bool retf = push_dummy_block(bucket_array, data_hash, ypc::oram::BucketSizeZ, 
          osf_header.item_num_each_batch, osf_header.item_size, crypto_ptr, pub_key);
      if(!retf) {
        LOG(ERROR) << "push_dummy_block fail!";
        return result;
      }
      
      oram_ntt::bucket_pkg_t bucket_pkg;
      bucket_pkg.set<oram_ntt::bucket>(bucket_array);
      bytes bucket_str = ypc::make_bytes<bytes>::for_package(bucket_pkg);

      // secondary encryption on the serialized bucket
      // in order to encrypt the mapping relationship between block_id and leaf_label
      bytes encrypted_bucket_bytes;
      uint32_t status = crypto_ptr->encrypt_message_with_prefix(
        pub_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
      if (status) {
        LOG(ERROR) << "encrypt_message_with_prefix fail: "
                    << stbox::status_string(status);
        return result;
      }

      ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
        (filepos, encrypted_bucket_bytes.data(), encrypted_bucket_bytes.size());
      if (ret != stbox::stx_status::success) {
        LOG(ERROR) << "write_convert_data_structure ocall fail!";
        return result;
      }

      filepos += encrypted_bucket_bytes.size();
      data_hash_array.push_back(data_hash);
      ++bucket_index;
    }



    std::vector<bytes> batch;
    std::vector<uint32_t> position_map_array(osf_header.block_num + 1, 0);


    LOG(INFO) << "write real blocks";
    // 2.2 write the bucket that contains both real and dummy blocks
    std::vector<oram_ntt::block_t> bucket_array;
    bytes data_hash;
    crypto_ptr->hash_256(bytes("Fidelius"), data_hash);
    int i = ypc::oram::BucketSizeZ;

    if(lastbucket_realblocknum != 0) {
      --real_bucket_num;
      push_dummy_block(bucket_array, data_hash, 
          ypc::oram::BucketSizeZ - lastbucket_realblocknum, 
          osf_header.item_num_each_batch, osf_header.item_size, crypto_ptr, pub_key);
      i = lastbucket_realblocknum;
    }

    bool break_flag = false;
    int j = 0;
    batch.clear();
    

    // 2.3 write buckets full of real blocks
    hpda::processor::internal::filter_impl<user_item_t> match3(
        &converter, [&](const user_item_t &v) {
          typename ypc::cast_obj_to_package<user_item_t>::type pt = v;
          auto item_data = ypc::make_bytes<bytes>::for_package(pt);

          if(break_flag) {
            return false;
          }

          batch.push_back(item_data);
          ++j;

          if(j == osf_header.item_num_each_batch) {
            
            bool retf = push_real_block(bucket_array, data_hash, block_id_value, bucket_index, 
              position_map_array, osf_header.level_num_L, batch, osf_header.batch_str_size, 
              osf_header.item_num_each_batch, osf_header.item_size, crypto_ptr, pub_key);
            if(!retf) {
              LOG(ERROR) << "push_dummy_block fail!";
              break_flag = true;
              return false;
            }
            
            --i;

            if(i == 0) {
              oram_ntt::bucket_pkg_t bucket_pkg;
              bucket_pkg.set<oram_ntt::bucket>(bucket_array);
              bytes bucket_str = ypc::make_bytes<bytes>::for_package(bucket_pkg);

              bytes encrypted_bucket_bytes;
              uint32_t status = crypto_ptr->encrypt_message_with_prefix(
                pub_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
              if (status) {
                LOG(ERROR) << "encrypt_message_with_prefix fail: "
                            << stbox::status_string(status);
                break_flag = true;
                return false;
              }

              if(osf_header.bucket_str_size != encrypted_bucket_bytes.size()) {
                osf_header.bucket_str_size = encrypted_bucket_bytes.size();
              }

              auto ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
                (filepos, encrypted_bucket_bytes.data(), encrypted_bucket_bytes.size());
              if (ret != stbox::stx_status::success) {
                LOG(ERROR) << "write_convert_data_structure ocall fail!";
                break_flag = true;
                return false;
              }


              filepos += encrypted_bucket_bytes.size();
              data_hash_array.push_back(data_hash);
              ++bucket_index;
              bucket_array.clear();
              crypto_ptr->hash_256(bytes("Fidelius"), data_hash);

              i = ypc::oram::BucketSizeZ;
            }
            
            batch.clear();
            j = 0;
          }
          
          return false;
        });
    match3.get_engine()->run();
    
    if(break_flag) {
      LOG(ERROR) << "write real blocks fail!";
      return result;
    }

    // 2.4 write the last data block
    // The number of valid rows in the last data block may be less than item_num_each_batch
    if(j > 0) {
      bool retf = push_real_block(bucket_array, data_hash, block_id_value, bucket_index, 
                position_map_array, osf_header.level_num_L, batch, osf_header.batch_str_size, 
                osf_header.item_num_each_batch, osf_header.item_size, crypto_ptr, pub_key);
      if(!retf) {
        LOG(ERROR) << "push_dummy_block fail!";
        break_flag = true;
        return result;
      }
      
      --i;

      if(i == 0) {
        oram_ntt::bucket_pkg_t bucket_pkg;
        bucket_pkg.set<oram_ntt::bucket>(bucket_array);
        bytes bucket_str = ypc::make_bytes<bytes>::for_package(bucket_pkg);

        bytes encrypted_bucket_bytes;
        uint32_t status = crypto_ptr->encrypt_message_with_prefix(
          pub_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
        if (status) {
          LOG(ERROR) << "encrypt_message_with_prefix fail: "
                      << stbox::status_string(status);
          break_flag = true;
          return result;
        }

        if(osf_header.bucket_str_size != encrypted_bucket_bytes.size()) {
          osf_header.bucket_str_size = encrypted_bucket_bytes.size();
        }

        auto ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
          (filepos, encrypted_bucket_bytes.data(), encrypted_bucket_bytes.size());
        if (ret != stbox::stx_status::success) {
          LOG(ERROR) << "write_convert_data_structure ocall fail!";
          break_flag = true;
          return result;
        }

        filepos += encrypted_bucket_bytes.size();
        data_hash_array.push_back(data_hash);
        ++bucket_index;
        bucket_array.clear();
        crypto_ptr->hash_256(bytes("Fidelius"), data_hash);
      }
    }


    LOG(INFO) << "write real blocks done";


    // 3. write position_map
    osf_header.position_map_filepos = filepos;
    oram_ntt::position_map_t position_map_pkg;
    position_map_pkg.set<oram_ntt::position_map>(position_map_array);
    bytes position_map_bytes = ypc::make_bytes<bytes>::for_package(position_map_pkg);
    ypc::bytes encrypted_position_map_bytes;
    uint32_t status = crypto_ptr->encrypt_message_with_prefix(
        pub_key, position_map_bytes, ypc::utc::crypto_prefix_arbitrary, encrypted_position_map_bytes);
    if (status != 0u) {
      LOG(ERROR) << "encrypt_message_with_prefix fail: "
                  << stbox::status_string(status);
      return result;
    }

    ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
      (filepos, encrypted_position_map_bytes.data(), encrypted_position_map_bytes.size());
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "write_convert_data_structure ocall fail!";
      return result;
    }
    filepos += encrypted_position_map_bytes.size();


    // 4. write merkle tree
    osf_header.merkle_tree_filepos = filepos;

    for(int i = (1 << osf_header.level_num_L) - 2; i >= 0; --i) {
      ypc::bytes k_hash = data_hash_array[i] + data_hash_array[2*i + 1];
      crypto_ptr->hash_256(k_hash, data_hash_array[i]);

      k_hash = data_hash_array[i] + data_hash_array[2*i + 2];
      crypto_ptr->hash_256(k_hash, data_hash_array[i]);
    }
    
    for(auto &data_hash : data_hash_array) {
      ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
        (filepos, data_hash.data(), data_hash.size());
      if (ret != stbox::stx_status::success) {
        LOG(ERROR) << "write_convert_data_structure ocall fail!";
        return result;
      }
      filepos += data_hash.size();
    }


    // 5. update and write osf_header
    osf_header.stash_filepos = filepos;
    osf_header.item_num_each_batch = 0;
    osf_header.item_size = 0;

    ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
      (0, (uint8_t *)&osf_header, sizeof(osf_header));
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "write_convert_data_structure ocall fail!";
      return result;
    }


    LOG(INFO) << "convert_parse2 done";

    return result;
  }


protected:
  ypc::data_source<bytes> *m_source;
};
