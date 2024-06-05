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


class convert_parser {
public:
  convert_parser() {}
  convert_parser(ypc::data_source<bytes> *source) : m_source(source){};

  inline bytes do_parse(const bytes &param) {
    bytes result;

    LOG(INFO) << "do convert_parse";
    ypc::to_type<bytes, user_item_t> converter(m_source);

    crypto_ptr_t crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
    bytes pub_key(param);
    

  
    LOG(INFO) << "create id_map";

    size_t batch_size = 0;
    size_t item_size = 0;
    uint64_t batch_num = 0; // the number of batch 
    uint64_t full_num = 0;
    uint64_t last_num = 0;
    uint64_t item_num = 0;

    std::vector<oram_ntt::id_map_pair> id_map_array;
    uint32_t batch_id = 1;

    hpda::processor::internal::filter_impl<user_item_t> match2(
        &converter, [&](const user_item_t &v) {
          if(item_size == 0) {
            typename ypc::cast_obj_to_package<user_item_t>::type pt = v;
            auto item_data = ypc::make_bytes<bytes>::for_package(pt);
            item_size = item_data.size();
          }

          std::string item_index_field = v.get<ZJHM>();

          input_buf_t item_index_field_pkg;
          item_index_field_pkg.set<input_buf>(item_index_field);
          bytes item_index_field_bytes = ypc::make_bytes<bytes>::for_package(item_index_field_pkg);
          bytes item_index_field_hash;
          crypto_ptr->hash_256(item_index_field_bytes, item_index_field_hash);

          std::shared_ptr<oram_ntt::id_map_pair> k_v(new oram_ntt::id_map_pair());
          k_v->set<oram_ntt::item_index_field_hash, oram_ntt::block_id>(item_index_field_hash, batch_id);
          id_map_array.push_back(*k_v);          

          ++item_num;
          
          batch_size += item_size;
          if (batch_size >= ypc::utc::max_item_size) {

            if(full_num == 0) {
              full_num = item_num;
            }

            item_num = 0;
            batch_size = 0;

            ++batch_id;
            ++batch_num;
          }

          return false;
        });
    match2.get_engine()->run();

    if(item_num > 0) {
      last_num = item_num;
      ++batch_num;
    }

    oram_ntt::id_map_t id_map_pkg;
    id_map_pkg.set<oram_ntt::id_map>(id_map_array);
    id_map_array.clear();
    bytes id_map_bytes = ypc::make_bytes<bytes>::for_package(id_map_pkg);
      

    LOG(INFO) << "write header";

    ypc::oram::header osf_header{};
    osf_header.block_num = batch_num;
    uint32_t real_bucket_num = ceil(static_cast<double>(osf_header.block_num) / ypc::oram::BucketSizeZ);
    osf_header.level_num_L = ceil(log2(real_bucket_num + 1)) - 1; 
    osf_header.bucket_num_N = (1 << (osf_header.level_num_L + 1)) - 1;
    osf_header.id_map_filepos = sizeof(osf_header);
    osf_header.oram_tree_filepos = osf_header.id_map_filepos + id_map_bytes.size();

    osf_header.item_size = item_size;
    if(full_num > 0) {
      osf_header.item_num_each_batch = full_num;
    } else {
      osf_header.item_num_each_batch = last_num;
    }

    auto ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
      (0, (uint8_t *)&osf_header, sizeof(osf_header));
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "write_convert_data_structure ocall fail!";
      return result;
    }
    
    LOG(INFO) << "write id map";

    int32_t id_map_size = 8000000;
    for(int i = 0; i <= id_map_bytes.size(); i += id_map_size) {
      if(i + id_map_size <= id_map_bytes.size()) {
        ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
          (osf_header.id_map_filepos + i, id_map_bytes.data() + i, id_map_size);
      } else {
        ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
          (osf_header.id_map_filepos + i, id_map_bytes.data() + i, id_map_bytes.size() - i);
      }
      
      if (ret != stbox::stx_status::success) {
        LOG(ERROR) << "write_convert_data_structure ocall fail!";
        return result;
      }
    }


    return result;
  }

protected:
  ypc::data_source<bytes> *m_source;
};
