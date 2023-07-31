#include "parsers/oram_parser.h"
#include "ypc/common/access_policy.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/core/status.h"
#include "ypc/core/ntjson.h"
#include "ypc/corecommon/package.h"
#include <glog/logging.h>

oram_parser::oram_parser(const input_param_t &param) : m_param(param) {}

oram_parser::~oram_parser() = default;

ypc::bytes construct_access_control_policy() {
  using ntt = ypc::nt<ypc::bytes>;
  ntt::access_list_package_t alp;
  alp.set<ntt::access_list_type>(ypc::utc::access_policy_blacklist);
  alp.set<ntt::access_list>(std::vector<ntt::access_item_t>());
  return ypc::make_bytes<ypc::bytes>::for_package(alp);
}

uint32_t oram_parser::oram_parse() {
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

  ret = feed_model();
  if (ret != 0u) {
    LOG(ERROR) << "feed_model got error " << ypc::status_string(ret);
    return ret;
  }

  ret = m_parser->begin_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "begin_parse_data_item, got error: "
               << ypc::status_string(ret);
    return ret;
  }
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

  return ypc::success;
}

uint32_t oram_parser::dump_result(const ypc::bytes &res) {
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

uint32_t oram_parser::feed_datasource() {
  auto input_data_var = m_param.get<input_data>();
  if (m_ptype.d.data_source_type == ypc::utc::noinput_datasource_parser) {
    return ypc::success;
  }

  if (m_ptype.d.data_source_type == ypc::utc::oram_sealed_datasource_parser) {
    if (input_data_var.empty()) {
      LOG(ERROR) << "missing input, require one input data source";
      return ypc::parser_missing_input;
    } if (input_data_var.size() > 1) {
      LOG(WARNING) << "only need 1 input, ignore other inputs";
    }
  }

  if (m_ptype.d.data_source_type == ypc::utc::single_sealed_datasource_parser) {
    if (input_data_var.empty()) {
      LOG(ERROR) << "missing input, require one input data source";
      return ypc::parser_missing_input;
    } if (input_data_var.size() > 1) {
      LOG(WARNING) << "only need 1 input, ignore other inputs";
    }
  }
  if (m_ptype.d.data_source_type == ypc::utc::multi_sealed_datasource_parser) {
    if (input_data_var.empty()) {
      LOG(ERROR) << "missing input, require at least one input data source";
      return ypc::parser_missing_input;
    }
  }

  auto epkey = m_param.get<dian_pkey>();
  auto param_var = m_param.get<ntt::param>();
  std::vector<ntt::oram_sealed_data_info_t> all_data_info;
  for (auto item : input_data_var) {
    auto url = item.get<input_data_url>();
    auto data_hash = item.get<input_data_hash>();
    auto sosf = std::make_shared<ypc::simple_oram_sealed_file>(url);
    // TODO:连续查的时候root hash一直在变化，总不能一次查询总是插入不同的root吧？
    m_data_sources.insert(std::make_pair(data_hash, sosf));
    sosf->reset();

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
    
    ntt::oram_sealed_data_info_t data_info;
    data_info.set<ntt::data_hash, ntt::pkey, ntt::tag, ntt::param_data, ntt::param_data_pkey>(
        data_hash, shu.get<shu_pkey>(), item.get<ntt::tag>(), 
        param_var.get<ntt::param_data>(), param_var.get<ntt::pkey>());
    all_data_info.push_back(data_info.make_copy());

  }

  ypc::bytes data_info_bytes;
  if (m_ptype.d.data_source_type == ypc::utc::oram_sealed_datasource_parser) {
    if (all_data_info.empty()) {
      LOG(ERROR) << "cannot get input data info";
      return ypc::parser_missing_input;
    }

    typename ypc::cast_obj_to_package<ntt::oram_sealed_data_info_t>::type oram =
        all_data_info[0];
    data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package(oram);
  }

  if (m_ptype.d.data_source_type == ypc::utc::single_sealed_datasource_parser) {
    if (all_data_info.empty()) {
      LOG(ERROR) << "cannot get input data info";
      return ypc::parser_missing_input;
    }

    typename ypc::cast_obj_to_package<ntt::sealed_data_info_t>::type single =
        all_data_info[0];
    data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package(single);
  }

  if (m_ptype.d.data_source_type == ypc::utc::multi_sealed_datasource_parser) { 
    data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package<
        typename ypc::cast_obj_to_package<ntt::multi_oram_sealed_data_info_t>::type,
        ntt::oram_sealed_data_info_vector>(all_data_info);
  }
  if (m_ptype.d.data_source_type == ypc::utc::raw_datasource_parser) {
    data_info_bytes = all_data_info[0].get<ntt::data_hash>();
  }

  auto ret = m_parser->init_data_source(data_info_bytes);
  if (ret != 0u) {
    LOG(ERROR) << "init_data_source got error " << ypc::status_string(ret);
    return ret;
  }
  return ypc::success;
}

uint32_t oram_parser::feed_model() {
  if (m_ptype.d.has_model == ypc::utc::no_model_parser) {
    return ypc::success;
  }

  auto model = m_param.get<ntt::model>();
  ypc::cast_obj_to_package<ntt::model_t>::type pkg = model;

  auto model_bytes = ypc::make_bytes<ypc::bytes>::for_package(pkg);
  uint32_t ret = m_parser->init_model(model_bytes);
  if (ret != 0u) {
    LOG(ERROR) << "init_model got error: " << ypc::status_string(ret);
    return ret;
  }

  return ypc::success;
}
uint32_t oram_parser::feed_param() { return ypc::success; }

uint32_t oram_parser::download_oram_params_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                        uint32_t *block_num, uint32_t *bucket_num_N, uint8_t *level_num_L, 
                        uint32_t *bucket_str_size, uint32_t *batch_str_size) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  bool ret = sosf->download_oram_params(block_num, bucket_num_N, level_num_L, bucket_str_size, batch_str_size);
  if(ret) {
    return stbox::stx_status::success;
  }
  
  // TODO:定义新的错误码
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::get_block_id_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                        uint32_t *block_id,
                                        const uint8_t *param_hash, uint32_t param_hash_size) {

  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  
  sosf->reset();
  ypc::bytes item_index_field_hash(param_hash, param_hash_size);
  bool ret = sosf->get_block_id(item_index_field_hash, block_id);
  if(ret) {
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::download_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                     uint8_t ** position_map, uint32_t *len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  sosf->reset();

  ypc::memref posmap;
  bool ret = sosf->download_position_map(posmap);
  if(ret) {
    *position_map = posmap.data();
    *len = posmap.size();
    return stbox::stx_status::success;
  }
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::update_position_map_OCALL(const uint8_t *data_hash, uint32_t hash_size, 
                                             uint8_t * position_map, uint32_t len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  sosf->reset();

  bool ret = sosf->update_position_map(position_map, len);
  if(ret) {
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::download_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                      uint32_t leaf, uint8_t ** encrpypted_path, uint32_t *len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  sosf->reset();

  ypc::memref en_path;
  bool ret = sosf->download_path(leaf, en_path);
  if(ret) {
    *encrpypted_path = en_path.data();
    *len = en_path.size();
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::download_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                        uint8_t ** stash, uint32_t *len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  sosf->reset();

  ypc::memref st;
  bool ret = sosf->download_stash(st);
  if(ret) {
    *stash = st.data();
    *len = st.size();
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::update_stash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                         uint8_t * stash, uint32_t len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }

  auto sosf = m_data_sources[hash];
  sosf->reset();

  bool ret = sosf->update_stash(stash, len);
  if(ret) {
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::upload_path_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                           uint32_t leaf, uint8_t * encrpypted_path, uint32_t len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  sosf->reset();

  bool ret = sosf->upload_path(leaf, encrpypted_path, len);
  if(ret) {
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::download_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                                 uint32_t leaf, uint8_t ** merkle_hash, uint32_t *len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  sosf->reset();

  ypc::memref me_hash;
  bool ret = sosf->download_merkle_hash(leaf, me_hash);
  if(ret) {
    *merkle_hash = me_hash.data();
    *len = me_hash.size();
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t oram_parser::update_merkle_hash_OCALL(const uint8_t *data_hash, uint32_t hash_size,
                                            uint32_t leaf, uint8_t * merkle_hash, uint32_t len) {
  auto hash = ypc::bytes(data_hash, hash_size);
  if (m_data_sources.find(hash) == m_data_sources.end()) {
    LOG(ERROR) << "data with hash: " << hash << " not found";
    return stbox::stx_status::data_source_not_found;
  }
  auto sosf = m_data_sources[hash];
  sosf->reset();

  bool ret = sosf->upload_path(leaf, merkle_hash, len);
  if(ret) {
    return stbox::stx_status::success;
  }
  
  return stbox::stx_status::sealed_file_reach_end;
}