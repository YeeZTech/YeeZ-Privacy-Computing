#include "parsers/parser.h"
#include "common/access_policy.h"
#include "corecommon/nt_cols.h"
#include "corecommon/package.h"
#include "ypc/ntjson.h"
#include "ypc/sealed_file.h"
#include "ypc/status.h"
#include <glog/logging.h>

parser::parser(const input_param_t &param) : m_param(param) {}

parser::~parser() {}

ypc::bytes construct_access_control_policy() {
  using ntt = ypc::nt<ypc::bytes>;
  ntt::access_list_package_t alp;
  alp.set<ntt::access_list_type>(ypc::utc::access_policy_blacklist);
  alp.set<ntt::access_list>(std::vector<ntt::access_item_t>());
  return ypc::make_bytes<ypc::bytes>::for_package(alp);
}

uint32_t parser::parse() {
  auto parser_enclave_path = m_param.get<parser_path>();
  auto keymgr_enclave_path = m_param.get<keymgr_path>();
  m_parser =
      std::make_shared<ypc::parser_sgx_module>(parser_enclave_path.c_str());
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
  if (shu_skey.size() > 0) {
    ret = m_keymgr->forward_private_key(
        shu_skey.data(), shu_skey.size(), epkey.data(), epkey.size(),
        ehash.data(), ehash.size(), shu_forward_sig.data(),
        shu_forward_sig.size());

    if (ret) {
      LOG(ERROR) << "forward_message got error " << ypc::status_string(ret);
      return ret;
    }
  }
  m_ptype.value = m_parser->get_parser_type();
  ypc::bytes actual_hash;
  ret = m_parser->get_enclave_hash(actual_hash);
  if (ret) {
    LOG(ERROR) << "get_enclave_hash got error " << ypc::status_string(ret);
    return ret;
  }

  if (actual_hash != ehash) {
    LOG(ERROR) << "parser hash is " << actual_hash << ", expect " << ehash;
    return ypc::parser_return_wrong_data_hash;
  }

  ret = feed_datasource();
  if (ret) {
    LOG(ERROR) << "feed_datasource got error " << ypc::status_string(ret);
    return ret;
  }

  ret = feed_model();
  if (ret) {
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
  if (ret) {
    LOG(ERROR) << "parse_data_item, got error: " << ypc::status_string(ret);
    return ret;
  }

  ret = m_parser->end_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "end_parse_data_item, got error: " << ypc::status_string(ret);
  }

  if (ret) {
    LOG(ERROR) << "do_parse got error " << ypc::status_string(ret);
    return ret;
  }
  LOG(INFO) << "parse done";

  ypc::bytes res;
  ret = m_parser->get_analyze_result(res);
  if (ret) {
    LOG(ERROR) << "get_analyze_result got error " << ypc::status_string(ret);
    return ret;
  }
  ret = dump_result(res);
  if (ret) {
    LOG(ERROR) << "dump_result got error " << ypc::status_string(ret);
    return ret;
  }

  return ypc::success;
}

uint32_t parser::dump_result(const ypc::bytes &res) {
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

uint32_t parser::feed_datasource() {
  auto input_data_var = m_param.get<input_data>();
  if (m_ptype.d.data_source_type == ypc::utc::noinput_datasource_parser) {
    return ypc::success;
  }

  if (m_ptype.d.data_source_type == ypc::utc::single_sealed_datasource_parser) {
    if (input_data_var.size() < 1) {
      LOG(ERROR) << "missing input, require one input data source";
      return ypc::parser_missing_input;
    } else if (input_data_var.size() > 1) {
      LOG(WARNING) << "only need 1 input, ignore other inputs";
    }
  }
  if (m_ptype.d.data_source_type == ypc::utc::multi_sealed_datasource_parser) {
    if (input_data_var.size() == 0) {
      LOG(ERROR) << "missing input, require at least one input data source";
      return ypc::parser_missing_input;
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

    auto shu = item.get<shu_info>();
    auto shu_skey = shu.get<ntt::encrypted_shu_skey>();
    auto shu_forward_sig = shu.get<ntt::shu_forward_signature>();
    auto target_enclave_hash = shu.get<enclave_hash>();
    auto ret = m_keymgr->forward_private_key(
        shu_skey.data(), shu_skey.size(), epkey.data(), epkey.size(),
        target_enclave_hash.data(), target_enclave_hash.size(),
        shu_forward_sig.data(), shu_forward_sig.size());
    if (ret) {
      LOG(ERROR) << "forward_message got error " << ypc::status_string(ret);
      return ret;
    }
    ntt::sealed_data_info_t data_info;
    data_info.set<ntt::data_hash, ntt::pkey, ntt::tag>(
        data_hash, shu.get<shu_pkey>(), item.get<ntt::tag>());
    all_data_info.push_back(data_info.make_copy());

  }

  ypc::bytes data_info_bytes;
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
        typename ypc::cast_obj_to_package<ntt::multi_sealed_data_info_t>::type,
        ntt::sealed_data_info_vector>(all_data_info);
  }
  if (m_ptype.d.data_source_type == ypc::utc::raw_datasource_parser) {
    data_info_bytes = all_data_info[0].get<ntt::data_hash>();
  }
  auto ret = m_parser->init_data_source(data_info_bytes);
  if (ret) {
    LOG(ERROR) << "init_data_source got error " << ypc::status_string(ret);
    return ret;
  }
  return ypc::success;
}

uint32_t parser::feed_model() {
  if (m_ptype.d.has_model == ypc::utc::no_model_parser) {
    return ypc::success;
  }

  auto model = m_param.get<ntt::model>();
  ypc::cast_obj_to_package<ntt::model_t>::type pkg = model;

  auto model_bytes = ypc::make_bytes<ypc::bytes>::for_package(pkg);
  uint32_t ret = m_parser->init_model(model_bytes);
  if (ret) {
    LOG(ERROR) << "init_model got error: " << ypc::status_string(ret);
    return ret;
  }

  return ypc::success;
}
uint32_t parser::feed_param() { return ypc::success; }

uint32_t parser::next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
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
  } else {
    LOG(ERROR) << "data with hash: " << hash << " reach end";
    return stbox::stx_status::sealed_file_reach_end;
  }
}

void parser::free_data_batch(uint8_t *data) { delete[] data; }

