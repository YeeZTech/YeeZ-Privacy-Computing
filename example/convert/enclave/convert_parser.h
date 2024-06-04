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
// #include "ypc/corecommon/crypto/gmssl.h"
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

// class convert_parser {
// public:
//   convert_parser() {}
//   convert_parser(ypc::data_source<bytes> *source) : m_source(source){};

//   inline bytes do_parse(const bytes &param) {
//     LOG(INFO) << "do parse";
//     ypc::to_type<bytes, user_item_t> converter(m_source);
//     // param must be serialized ntpackage
//     auto pkg = ypc::make_package<input_buf_t>::from_bytes(param);
//     int counter = 0;
//     hpda::processor::internal::filter_impl<user_item_t> match(
//         &converter, [&](const user_item_t &v) {
//           counter++;
//           std::string zjhm = v.get<ZJHM>();
//           if (zjhm == pkg.get<input_buf>()) {
//             return true;
//           }
//           return false;
//         });

//     hpda::output::internal::memory_output_impl<user_item_t> mo(&match);
//     mo.get_engine()->run();
//     LOG(INFO) << "do parse done";

//     LOG(INFO) << "parse test done";

//     bytes result;
//     for (auto it : mo.values()) {
//       stbox::printf("found\n");
//       result += it.get<XM>();
//       result += " : ";
//       result += it.get<ZJHM>();
//       result += " .";
//     }
//     return result;
//   }

//   inline bool merge_parse_result(const std::vector<bytes> &block_result,
//                                  const bytes &param, bytes &result) {
//     bytes s;
//     for (auto k : block_result) {
//       s = s + k;
//     }
//     result = s;
//     return false;
//   }

// protected:
//   ypc::data_source<bytes> *m_source;
// };


class convert_parser {
public:
  convert_parser() {}
  convert_parser(ypc::data_source<bytes> *source) : m_source(source){};

  inline bytes do_parse(const bytes &param) {
    bytes result;


    LOG(INFO) << "do convert_parse";
    // TODO:converter这步的转换是必要的吗？
    ypc::to_type<bytes, user_item_t> converter(m_source);
    // hpda::processor::internal::split_impl<user_item_t> split(&converter);


    // TODO:临时，待删除
    // crypto_ptr_t crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
    crypto_ptr_t crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
    // bytes pub_key("7cb409d496e484cb0b745ac7c7645a953c33bb82281cccbd580130ff543211554254f71f7aa85546ba8ff4f88b981e750e0ba6c0c92b99a01caeaee625a2ac6c");
    bytes pub_key(param);


    // 0. 获取数据提供方的枢公钥、加密算法

    // param must be serialized ntpackage
    // auto pkg = ypc::make_package<oram_ntt::convert_param_pkg_t>::from_bytes(param);
    
    // bytes crypto_type = pkg.get<oram_ntt::crypto>();
    // bytes pub_key = pkg.get<oram_ntt::public_key>();
    // auto crypto_type_pkg = ypc::make_package<input_buf_t>::from_bytes(crypto_type);

    // crypto_ptr_t crypto_ptr;
    // if (crypto_type_pkg.get<input_buf>() == "stdeth") {
    //   crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
    // } else if (crypto_type_pkg.get<input_buf>() == "gmssl") {
    //   crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
    // } else {
    //   throw std::runtime_error("Unsupperted crypto type!");
    // }



    // auto crypto_type_pkg = ypc::make_package<input_buf_t>::from_bytes(param);
    // LOG(INFO) << "crypto_type_pkg.get<input_buf>() : " << crypto_type_pkg.get<input_buf>();
    // crypto_ptr_t crypto_ptr;
    // if (crypto_type_pkg.get<input_buf>() == "stdeth") {
    //   crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
    // } else if (crypto_type_pkg.get<input_buf>() == "gmssl") {
    //   crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
    // } else {
    //   throw std::runtime_error("Unsupperted crypto type!");
    // }
    
    
  
    

    size_t batch_size = 0;
    size_t item_size = 0;
    // the number of batch 
    uint64_t batch_num = 0;

    uint64_t full_num = 0;
    uint64_t last_num = 0;

    
    // 2. build id map
    LOG(INFO) << "build id_map";

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


          // TODO:注意建立的索引项只能写死，因此本文件可以用python脚本生成后，再编译
          std::string item_index_field = v.get<ZJHM>();

          // LOG(INFO) << "ZJHM: " << item_index_field;

          input_buf_t item_index_field_pkg;
          item_index_field_pkg.set<input_buf>(item_index_field);
          bytes item_index_field_bytes = ypc::make_bytes<bytes>::for_package(item_index_field_pkg);
          bytes item_index_field_hash;
          crypto_ptr->hash_256(item_index_field_bytes, item_index_field_hash);

          oram_ntt::id_map_pair k_v;
          k_v.set<oram_ntt::item_index_field_hash, oram_ntt::block_id>(item_index_field_hash, batch_id);
          id_map_array.push_back(k_v);

          ++item_num;

          
          batch_size += item_size;
          if (batch_size >= ypc::utc::max_item_size) {

            // LOG(INFO) << "item_num :" << item_num;
            // LOG(INFO) << "batch_id :" << batch_id;

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
    // match2.get_engine()->run();
    hpda::output::internal::memory_output_impl<user_item_t> mo(&match2);
    mo.get_engine()->run();

    
    LOG(INFO) << "item_num :" << item_num;

    if(item_num > 0) {
      last_num = item_num;

      // item_num = 0;
      // batch_size = 0;
      ++batch_num;
    }

    LOG(INFO) << "batch_num :" << batch_num;
    LOG(INFO) << "batch_id :" << batch_id;


    oram_ntt::id_map_t id_map_pkg;
    id_map_pkg.set<oram_ntt::id_map>(id_map_array);
    bytes id_map_bytes = ypc::make_bytes<bytes>::for_package(id_map_pkg);
      


    // 3.write header, id map
    LOG(INFO) << "write header and id map";

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


    LOG(INFO) << "osf_header.block_num :" << osf_header.block_num;
    LOG(INFO) << "osf_header.bucket_num_N :" << osf_header.bucket_num_N;
    LOG(INFO) << "osf_header.level_num_L :" << osf_header.level_num_L;
    LOG(INFO) << "osf_header.bucket_str_size :" << osf_header.bucket_str_size;
    LOG(INFO) << "osf_header.batch_str_size :" << osf_header.batch_str_size;
    LOG(INFO) << "osf_header.id_map_filepos :" << osf_header.id_map_filepos;
    LOG(INFO) << "osf_header.oram_tree_filepos :" << osf_header.oram_tree_filepos;
    LOG(INFO) << "osf_header.position_map_filepos :" << osf_header.position_map_filepos;
    LOG(INFO) << "osf_header.merkle_tree_filepos :" << osf_header.merkle_tree_filepos;
    LOG(INFO) << "osf_header.stash_filepos :" << osf_header.stash_filepos;
    LOG(INFO) << "osf_header.stash_size :" << osf_header.stash_size;
    LOG(INFO) << "osf_header.item_num_each_batch :" << osf_header.item_num_each_batch;
    LOG(INFO) << "osf_header.item_size :" << osf_header.item_size;


    // LOG(INFO) << "sizeof(osf_header) : " << sizeof(osf_header);
    // LOG(INFO) << "id_map_bytes.size() :" << id_map_bytes.size();
    // LOG(INFO) << "full_num :" << full_num;
    // LOG(INFO) << "last_num :" << last_num;






    LOG(INFO) << "write header";

    auto ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
      (0, (uint8_t *)&osf_header, sizeof(osf_header));
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "write_convert_data_structure ocall fail!";
      return result;
    }
    
    LOG(INFO) << "write id map";

    ret = stbox::ocall_cast<uint32_t>(write_convert_data_structure)
      (osf_header.id_map_filepos, id_map_bytes.data(), id_map_bytes.size());
    if (ret != stbox::stx_status::success) {
      LOG(ERROR) << "write_convert_data_structure ocall fail!";
      return result;
    }




    // ypc::oram::header t_osf_header{};

    // ret = stbox::ocall_cast<uint32_t>(download_convert_params_ocall)
    //   (&t_osf_header.block_num, &t_osf_header.oram_tree_filepos, &t_osf_header.item_num_each_batch, &t_osf_header.item_size);
    // if (ret != stbox::stx_status::success) {
    //   LOG(ERROR) << "download_convert_params_ocall fail!";
    //   return result;
    // }


    // LOG(INFO) << "t_osf_header.block_num :" << t_osf_header.block_num;
    // LOG(INFO) << "t_osf_header.oram_tree_filepos :" << t_osf_header.oram_tree_filepos;
    // LOG(INFO) << "t_osf_header.item_num_each_batch :" << t_osf_header.item_num_each_batch;
    // LOG(INFO) << "t_osf_header.item_size :" << t_osf_header.item_size;


    return result;
  }

  inline bool merge_parse_result(const std::vector<bytes> &block_result,
                                 const bytes &param, bytes &result) {
    bytes s;
    for (auto k : block_result) {
      s = s + k;
    }
    result = s;
    return false;
  }

protected:
  ypc::data_source<bytes> *m_source;
};
