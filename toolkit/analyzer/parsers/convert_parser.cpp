#include "parsers/convert_parser.h"
#include "ypc/common/access_policy.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/core/status.h"
#include "ypc/core/ntjson.h"
#include "ypc/corecommon/package.h"
#include <glog/logging.h>




convert_parser::convert_parser(const input_param_t &param) : m_param(param) {}

convert_parser::~convert_parser() = default;

ypc::bytes construct_access_control_policy() {
  using ntt = ypc::nt<ypc::bytes>;
  ntt::access_list_package_t alp;
  alp.set<ntt::access_list_type>(ypc::utc::access_policy_blacklist);
  alp.set<ntt::access_list>(std::vector<ntt::access_item_t>());
  return ypc::make_bytes<ypc::bytes>::for_package(alp);
}

uint32_t convert_parser::convert_parse() {
  auto parser_enclave_path = m_param.get<parser_path>();
#ifdef DEBUG
  LOG(INFO) << "parser enclave path: " << parser_enclave_path;
#endif
  auto keymgr_enclave_path = m_param.get<keymgr_path>();
  m_parser =
      std::make_shared<ypc::parser_sgx_module>(parser_enclave_path.c_str());
#ifdef DEBUG
  LOG(INFO) << "keymgr enclave path: " << keymgr_enclave_path;
#endif
  m_keymgr = std::make_shared<keymgr_sgx_module>(keymgr_enclave_path.c_str());

  ypc::bytes policy = construct_access_control_policy();
  m_keymgr->set_access_control_policy(policy);
  LOG(INFO) << "initializing parser/keymgr module done";

  // TODO:不需要数据使用方的
  auto epkey = m_param.get<dian_pkey>();
  auto ehash = m_param.get<parser_enclave_hash>();
  auto shu_skey = m_param.get<shu_info>().get<ntt::encrypted_shu_skey>();
  auto shu_forward_sig =
      m_param.get<shu_info>().get<ntt::shu_forward_signature>();
  
  uint32_t ret = 0;
  if (!shu_skey.empty()) {
    LOG(INFO) << "keymgr_enclave_path: " << keymgr_enclave_path;

    ret = m_keymgr->forward_private_key(
        shu_skey.data(), shu_skey.size(), epkey.data(), epkey.size(),
        ehash.data(), ehash.size(), shu_forward_sig.data(),
        shu_forward_sig.size());
    if (ret != 0u) {
      LOG(ERROR) << "forward_message got error " << ypc::status_string(ret);
      return ret;
    }
  }




  m_ptype.value = m_parser->get_parser_type();
  ypc::bytes actual_hash;
  ret = m_parser->get_enclave_hash(actual_hash);
  if (ret != 0u) {
    LOG(ERROR) << "get_enclave_hash got error " << ypc::status_string(ret);
    return ret;
  }

  if (actual_hash != ehash) {
    LOG(ERROR) << "parser hash is " << actual_hash << ", expect " << ehash;
    return ypc::parser_return_wrong_data_hash;
  }

  ret = feed_datasource();
  if (ret != 0u) {
    LOG(ERROR) << "feed_datasource got error " << ypc::status_string(ret);
    return ret;
  }

  ret = m_parser->begin_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "begin_parse_data_item, got error: "
               << ypc::status_string(ret);
    return ret;
  }

  // TODO:传入加密算法和数据提供方的公钥
  auto param_var = m_param.get<ntt::param>();
  typename ypc::cast_obj_to_package<ntt::param_t>::type param_pkg = param_var;
  auto param_bytes = ypc::make_bytes<ypc::bytes>::for_package(param_pkg);
  ret = m_parser->parse_data_item(param_bytes.data(), param_bytes.size());
  if (ret != 0u) {
    LOG(ERROR) << "parse_data_item, got error: " << ypc::status_string(ret);
    return ret;
  }

  ret = m_parser->end_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "end_parse_data_item, got error: " << ypc::status_string(ret);
  }

  if (ret != 0u) {
    LOG(ERROR) << "do_parse got error " << ypc::status_string(ret);
    return ret;
  }
  LOG(INFO) << "parse done";

  ypc::bytes res;
  ret = m_parser->get_analyze_result(res);
  if (ret != 0u) {
    LOG(ERROR) << "get_analyze_result got error " << ypc::status_string(ret);
    return ret;
  }
  ret = dump_result(res);
  if (ret != 0u) {
    LOG(ERROR) << "dump_result got error " << ypc::status_string(ret);
    return ret;
  }

  m_oram_sealed_file.close();

  // TODO:元数据描述文件修改，或者创建ORAM元数据描述文件

  return ypc::success;
}

uint32_t convert_parser::dump_result(const ypc::bytes &res) {
  if (m_ptype.d.result_type == ypc::utc::onchain_result_parser) {
    auto pkg =
        ypc::make_package<ntt::onchain_result_package_t>::from_bytes(res);
    typename ypc::cast_package_to_obj<ntt::onchain_result_package_t>::type p =
        pkg;
    m_result_str = ypc::ntjson::to_json(p);
  } else if (m_ptype.d.result_type == ypc::utc::offchain_result_parser) {
    auto pkg =
        ypc::make_package<ntt::offchain_result_package_t>::from_bytes(res);
    typename ypc::cast_package_to_obj<ntt::offchain_result_package_t>::type p =
        pkg;
    m_result_str = ypc::ntjson::to_json(p);

  } else if (m_ptype.d.result_type == ypc::utc::local_result_parser) {
    m_result_str = std::string((const char *)res.data(), res.size());
  } else if (m_ptype.d.result_type == ypc::utc::forward_result_parser) {
    auto pkg = ypc::make_package<typename ypc::cast_obj_to_package<
        ntt::forward_result_t>::type>::from_bytes(res);

    typename ypc::cast_package_to_obj<ntt::forward_result_t>::type p = pkg;
    m_result_str = ypc::ntjson::to_json(p);
  } else {
    return ypc::parser_unknown_result;
  }
  return ypc::success;
}

uint32_t convert_parser::feed_datasource() {
  auto input_data_var = m_param.get<input_data>();
  if (m_ptype.d.data_source_type == ypc::utc::noinput_datasource_parser) {
    return ypc::success;
  }

  // TODO:只有一种类型的parser的话，就不需要区分了
  if (m_ptype.d.data_source_type == ypc::utc::convert_sealed_datasource_parser) {
    if (input_data_var.empty()) {
      LOG(ERROR) << "missing input, require one input data source";
      return ypc::parser_missing_input;
    } if (input_data_var.size() > 1) {
      LOG(WARNING) << "only need 1 input, ignore other inputs";
    }
  }

  auto epkey = m_param.get<dian_pkey>();
  std::vector<ntt::sealed_data_info_t> all_data_info;
  for (auto item : input_data_var) {
    auto url = item.get<input_data_url>();
    auto data_hash = item.get<input_data_hash>();
    auto ssf = std::make_shared<ypc::simple_sealed_file>(url, true);
    m_data_sources.insert(std::make_pair(data_hash, ssf));
    ssf->reset_read();
  
    m_oram_sealed_file_path = url + ".oram";
    m_oram_sealed_file = std::fstream(m_oram_sealed_file_path, std::ios::in | std::ios::out | std::ios::binary);
    if(!m_oram_sealed_file.is_open()) {
      m_oram_sealed_file.open(m_oram_sealed_file_path, std::ios::out);
      if(!m_oram_sealed_file.is_open()) {
        LOG(ERROR) << "Failed to create oram sealed file: " + m_oram_sealed_file_path;
        return ypc::parser_oram_sealed_file_cannot_create;
      }
    }

    auto shu = item.get<shu_info>();
    auto shu_skey = shu.get<ntt::encrypted_shu_skey>();
    auto shu_forward_sig = shu.get<ntt::shu_forward_signature>();
    auto target_enclave_hash = shu.get<enclave_hash>();
    auto ret = m_keymgr->forward_private_key(
        shu_skey.data(), shu_skey.size(), epkey.data(), epkey.size(),
        target_enclave_hash.data(), target_enclave_hash.size(),
        shu_forward_sig.data(), shu_forward_sig.size());
    if (ret != 0u) {
      LOG(ERROR) << "forward_message got error " << ypc::status_string(ret);
      return ret;
    }
    ntt::sealed_data_info_t data_info;
    data_info.set<ntt::data_hash, ntt::pkey, ntt::tag>(
        data_hash, shu.get<shu_pkey>(), item.get<ntt::tag>());
    all_data_info.push_back(data_info.make_copy());

  }

  // TODO:只有一种类型的parser的话，就不需要区分了
  ypc::bytes data_info_bytes;
  if (m_ptype.d.data_source_type == ypc::utc::convert_sealed_datasource_parser) {
    if (all_data_info.empty()) {
      LOG(ERROR) << "cannot get input data info";
      return ypc::parser_missing_input;
    }

    typename ypc::cast_obj_to_package<ntt::sealed_data_info_t>::type single =
        all_data_info[0];
    data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package(single);
  }

  auto ret = m_parser->init_data_source(data_info_bytes);
  if (ret != 0u) {
    LOG(ERROR) << "init_data_source got error " << ypc::status_string(ret);
    return ret;
  }
  LOG(INFO) << "end init_data_source ";
  return ypc::success;
}

// uint32_t convert_parser::feed_model() {
//   if (m_ptype.d.has_model == ypc::utc::no_model_parser) {
//     return ypc::success;
//   }

//   auto model = m_param.get<ntt::model>();
//   ypc::cast_obj_to_package<ntt::model_t>::type pkg = model;

//   auto model_bytes = ypc::make_bytes<ypc::bytes>::for_package(pkg);
//   uint32_t ret = m_parser->init_model(model_bytes);
//   if (ret != 0u) {
//     LOG(ERROR) << "init_model got error: " << ypc::status_string(ret);
//     return ret;
//   }

//   return ypc::success;
// }

// uint32_t convert_parser::feed_param() { return ypc::success; }

uint32_t convert_parser::write_convert_data_structure(int64_t filepos, const uint8_t * id_map_bytes, uint32_t len) {
  
  // struct header {
  //   uint32_t block_num;
  //   uint32_t bucket_num_N;
  //   uint8_t level_num_L;
  //   uint32_t bucket_str_size;
  //   uint32_t batch_str_size;
  //   long int id_map_filepos;
  //   long int oram_tree_filepos;
  //   long int position_map_filepos;
  //   long int merkle_tree_filepos;
  //   long int stash_filepos;
  //   uint64_t stash_size;
  // };

  // header osf_header{};
  // osf_header.block_num = batch_num;
  // uint32_t real_bucket_num = ceil(static_cast<double>(osf_header.block_num) / ypc::oram::BucketSizeZ);
  // osf_header.level_num_L = ceil(log2(real_bucket_num + 1)) - 1; 
  // osf_header.bucket_num_N = (1 << (osf_header.level_num_L + 1)) - 1;
  // osf_header.id_map_filepos = sizeof(osf_header);
  // osf_header.oram_tree_filepos = osf_header.id_map_filepos + len;

  // m_oram_sealed_file.write((char *)&osf_header, sizeof(osf_header));
  

  // TODO:写之前打开m_oram_sealed_file
  // std::fstream m_oram_sealed_file = std::fstream(m_oram_sealed_file_path, std::ios::out | std::ios::binary);
  // if(!m_oram_sealed_file.is_open()) {
  //   LOG(ERROR) << "Failed to create oram sealed file: " + m_oram_sealed_file_path;
  //   return ypc::parser_oram_sealed_file_cannot_create;
  // }

  try {
    m_oram_sealed_file.seekp(filepos, m_oram_sealed_file.beg);
    m_oram_sealed_file.write((char *)id_map_bytes, len);    
  } catch (const std::exception &e) {
    LOG(ERROR) << "write_convert_data_structure got error: " << e.what();
    return stbox::stx_status::convert_parser_error;
  }

  // TODO:写之后关闭m_oram_sealed_file
  // m_oram_sealed_file.close();


  // ypc::oram::header osf_header{};
  
  // try {
  //   m_oram_sealed_file.seekg(0, m_oram_sealed_file.beg);
  //   m_oram_sealed_file.read((char *)&osf_header, 88);
  //   LOG(INFO) << "osf_header.block_num :" << osf_header.block_num;
  //   LOG(INFO) << "osf_header.bucket_num_N :" << osf_header.bucket_num_N;
  //   LOG(INFO) << "osf_header.level_num_L :" << osf_header.level_num_L;
  //   LOG(INFO) << "osf_header.bucket_str_size :" << osf_header.bucket_str_size;
  //   LOG(INFO) << "osf_header.batch_str_size :" << osf_header.batch_str_size;
  //   LOG(INFO) << "osf_header.id_map_filepos :" << osf_header.id_map_filepos;
  //   LOG(INFO) << "osf_header.oram_tree_filepos :" << osf_header.oram_tree_filepos;
  //   LOG(INFO) << "osf_header.position_map_filepos :" << osf_header.position_map_filepos;
  //   LOG(INFO) << "osf_header.merkle_tree_filepos :" << osf_header.merkle_tree_filepos;
  //   LOG(INFO) << "osf_header.stash_filepos :" << osf_header.stash_filepos;
  //   LOG(INFO) << "osf_header.stash_size :" << osf_header.stash_size;
  //   LOG(INFO) << "osf_header.item_num_each_batch :" << osf_header.item_num_each_batch;
  //   LOG(INFO) << "osf_header.item_size :" << osf_header.item_size;
  // } catch (const std::exception &e) {
  //   LOG(ERROR) << "write_convert_data_structure got error: " << e.what();
  //   return stbox::stx_status::convert_parser_error;
  // }

  return stbox::stx_status::success;
  
  // return stbox::stx_status::oram_sealed_file_error;
}









// uint32_t convert_parser::download_convert_params_ocall(int64_t filepos, uint8_t **convert_header, uint32_t len) {




uint32_t convert_parser::download_convert_params_ocall(uint32_t *block_num, long int *oram_tree_filepos, 
    uint64_t *item_num_each_batch, uint64_t *item_size) {

  // std::fstream m_oram_sealed_file = std::fstream(m_oram_sealed_file_path, std::ios::in | std::ios::binary);
  // if(!m_oram_sealed_file.is_open()) {
  //   LOG(ERROR) << "Failed to create oram sealed file: " + m_oram_sealed_file_path;
  //   return ypc::parser_oram_sealed_file_cannot_create;
  // }

  

  // m_oram_sealed_file.seekg(0, std::ios::end); // 将文件指针移到文件末尾
  // std::streampos fileSize = m_oram_sealed_file.tellg(); // 获取当前文件指针位置，即文件长度
  // LOG(INFO) << "convert_parser::download_convert_params_ocall fileSize : " << fileSize;

  

  ypc::oram::header osf_header{};
  
  try {

    m_oram_sealed_file.seekg(0, m_oram_sealed_file.beg);
    m_oram_sealed_file.read((char *)&osf_header, sizeof(osf_header));

    *block_num = osf_header.block_num;
    *oram_tree_filepos = osf_header.oram_tree_filepos;
    *item_num_each_batch = osf_header.item_num_each_batch;
    *item_size = osf_header.item_size;
    
    
    // uint32_t level_num_L = osf_header.level_num_L;
    // LOG(INFO) << "osf_header.block_num :" << osf_header.block_num;
    // LOG(INFO) << "osf_header.bucket_num_N :" << osf_header.bucket_num_N;
    // LOG(INFO) << "osf_header.level_num_L :" << level_num_L;
    // LOG(INFO) << "osf_header.bucket_str_size :" << osf_header.bucket_str_size;
    // LOG(INFO) << "osf_header.batch_str_size :" << osf_header.batch_str_size;
    // LOG(INFO) << "osf_header.id_map_filepos :" << osf_header.id_map_filepos;
    // LOG(INFO) << "osf_header.oram_tree_filepos :" << osf_header.oram_tree_filepos;
    // LOG(INFO) << "osf_header.position_map_filepos :" << osf_header.position_map_filepos;
    // LOG(INFO) << "osf_header.merkle_tree_filepos :" << osf_header.merkle_tree_filepos;
    // LOG(INFO) << "osf_header.stash_filepos :" << osf_header.stash_filepos;
    // LOG(INFO) << "osf_header.stash_size :" << osf_header.stash_size;
    // LOG(INFO) << "osf_header.item_num_each_batch :" << osf_header.item_num_each_batch;
    // LOG(INFO) << "osf_header.item_size :" << osf_header.item_size;

    // *convert_header = (uint8_t *)&osf_header;


  } catch (const std::exception &e) {
    LOG(ERROR) << "download_convert_params_ocall got error: " << e.what();
    return stbox::stx_status::convert_parser_error;
  }

  // m_oram_sealed_file.close();

  return stbox::stx_status::success;
}

// uint32_t convert_parser::download_convert_params_ocall(uint32_t *block_num, long int *oram_tree_filepos, 
//     uint64_t *item_num_each_batch, size_t *item_size) {
//   auto sosf = std::make_shared<ypc::simple_oram_sealed_file>(m_oram_sealed_file_path);
//   sosf->open_for_read();
//   bool ret = sosf->download_convert_params(block_num, oram_tree_filepos, item_num_each_batch, item_size);
//   if(ret) {
//     return stbox::stx_status::success;
//   }
  
//   return stbox::stx_status::convert_parser_error;
// }


       


uint32_t convert_parser::next_data_batch_convert(const uint8_t *data_hash, uint32_t hash_size,
                                 uint8_t **data, uint32_t *len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto ssf = m_data_sources[hash];
  ypc::memref b;
  bool ret = ssf->next_item(b);
  if (ret) {
    *data = b.data();
    *len = b.size();
    return stbox::stx_status::success;
  }     return stbox::stx_status::sealed_file_reach_end;
 
}

void convert_parser::free_data_batch_convert(uint8_t *data) { delete[] data; }
  
  
