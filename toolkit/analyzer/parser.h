#pragma once
#include "iodef.h"
#include "modules/module_parser_interface.h"
#include "ypc/common/access_policy.h"
#include "ypc/common/parser_type.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/status.h"
#include "ypc/keymgr/default/keymgr_bridge.h"
#include "ypc/keymgr/default/keymgr_sgx_module.h"
#include <dlfcn.h>
#include <memory>
#include <unordered_map>

struct parser_module_loader {
  typedef parser_module_interface *(*create_instance_t)(const char *);

  static parser_module_interface *create_module_instance(const char *lib_module,
                                                         const char *mod_path) {
    void *handle = dlopen(lib_module, RTLD_LAZY);
    if (!handle) {
      std::cerr << "Cannot open library: " << dlerror() << '\n';
      return nullptr;
    }
    create_instance_t func_create_instance =
        (create_instance_t)dlsym(handle, "create_instance");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
      std::cerr << "Cannot load symbol 'create_instance': " << dlsym_error
                << '\n';
      dlclose(handle);
      return nullptr;
    }
    return func_create_instance(mod_path);
  }
};

class parser {
public:
  parser(const input_param_t &param) : m_param(param) {
    size_t s = ypc::simple_sealed_file::blockfile_t::BlockSizeLimit;
    m_mem_buf.reset(new char[s]);
    m_mem_buf_size = s;
  }
  virtual ~parser() {}

  virtual uint32_t parse(const std::string &lib_module) {
    auto parser_enclave_path = m_param.get<parser_path>();
#ifdef DEBUG
    LOG(INFO) << "parser enclave path: " << parser_enclave_path;
#endif
    auto keymgr_enclave_path = m_param.get<keymgr_path>();
    LOG(INFO) << "lib module: " << lib_module;
    LOG(INFO) << "parser enclave path: " << parser_enclave_path;
    set_module(lib_module.c_str(), parser_enclave_path.c_str());
#ifdef DEBUG
    LOG(INFO) << "keymgr enclave path: " << keymgr_enclave_path;
#endif
    auto keymgr_module =
        std::make_shared<ypc::keymgr_sgx_module>(keymgr_enclave_path.c_str());
    m_keymgr_parser = std::make_shared<ypc::keymgr_parser>(keymgr_module);
    ypc::init_sgx_keymgr(m_keymgr_parser->keymgr());

    ypc::bytes policy = construct_access_control_policy();
    m_keymgr_parser->keymgr()->set_access_control_policy(policy);
    LOG(INFO) << "initializing parser/keymgr module done";

    auto epkey = m_param.get<dian_pkey>();
    auto ehash = m_param.get<parser_enclave_hash>();
    auto shu_skey = m_param.get<shu_info>().get<ntt::encrypted_shu_skey>();
    auto shu_forward_sig =
        m_param.get<shu_info>().get<ntt::shu_forward_signature>();

    uint32_t ret = 0;
    if (!shu_skey.empty()) {
      LOG(INFO) << "keymgr_enclave_path: " << keymgr_enclave_path;
      ret = m_keymgr_parser->keymgr()->forward_private_key(
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
      LOG(ERROR) << "end_parse_data_item, got error: "
                 << ypc::status_string(ret);
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

  virtual uint32_t next_data_batch(const uint8_t *hash_and_pkey,
                                   uint32_t hash_and_pkey_size, uint8_t **data,
                                   uint32_t *len) {
    auto all = ypc::bytes(hash_and_pkey, hash_and_pkey_size);
    if (m_data_sources.find(all) == m_data_sources.end()) {
      auto hash = ypc::bytes(hash_and_pkey, 32);
      auto pkey = ypc::bytes(hash_and_pkey + 32, hash_and_pkey_size - 32);
      LOG(ERROR) << "data with hash: " << hash << ", pkey: " << pkey
                 << " not found";
      return stbox::stx_status::data_source_not_found;
    }
    auto ssf = m_data_sources[all];
    size_t s;
    bool ret = ssf->next_item(m_mem_buf.get(), m_mem_buf_size, s) ==
               ypc::simple_sealed_file::blockfile_t::succ;
    if (ret && 0u != s) {
      *data = (uint8_t *)m_mem_buf.get();
      *len = s;
      return stbox::stx_status::success;
    }
    return stbox::stx_status::sealed_file_reach_end;
  }

  inline const std::string &get_result_str() const { return m_result_str; }

protected:
  uint32_t feed_datasource() {
    auto input_data_var = m_param.get<input_data>();
    if (m_ptype.d.data_source_type == ypc::utc::noinput_datasource_parser) {
      return ypc::success;
    }

    if (m_ptype.d.data_source_type ==
        ypc::utc::single_sealed_datasource_parser) {
      if (input_data_var.empty()) {
        LOG(ERROR) << "missing input, require one input data source";
        return ypc::parser_missing_input;
      }
      if (input_data_var.size() > 1) {
        LOG(WARNING) << "only need 1 input, ignore other inputs";
      }
    }
    if (m_ptype.d.data_source_type ==
        ypc::utc::multi_sealed_datasource_parser) {
      if (input_data_var.empty()) {
        LOG(ERROR) << "missing input, require at least one input data source";
        return ypc::parser_missing_input;
      }
    }

    auto epkey = m_param.get<dian_pkey>();
    std::vector<ntt::sealed_data_info_t> all_data_info;
    for (auto item : input_data_var) {
      auto url = item.get<input_data_url>();
      auto data_hash = item.get<input_data_hash>();
      auto shu = item.get<shu_info>();
      auto spkey = shu.get<shu_pkey>();

      auto ssf = std::make_shared<ypc::simple_sealed_file>(url, true);
      m_data_sources.insert(std::make_pair(data_hash + spkey, ssf));
      ssf->reset_read();

      auto shu_skey = shu.get<ntt::encrypted_shu_skey>();
      auto shu_forward_sig = shu.get<ntt::shu_forward_signature>();
      auto target_enclave_hash = shu.get<enclave_hash>();
      auto ret = m_keymgr_parser->keymgr()->forward_private_key(
          shu_skey.data(), shu_skey.size(), epkey.data(), epkey.size(),
          target_enclave_hash.data(), target_enclave_hash.size(),
          shu_forward_sig.data(), shu_forward_sig.size());
      if (ret != 0u) {
        LOG(ERROR) << "forward_message got error " << ypc::status_string(ret);
        return ret;
      }
      ntt::sealed_data_info_t data_info;
      data_info.set<ntt::data_hash, ntt::pkey, ntt::tag>(data_hash, spkey,
                                                         item.get<ntt::tag>());
      all_data_info.push_back(data_info.make_copy());
    }

    ypc::bytes data_info_bytes;
    if (m_ptype.d.data_source_type ==
        ypc::utc::single_sealed_datasource_parser) {
      if (all_data_info.empty()) {
        LOG(ERROR) << "cannot get input data info";
        return ypc::parser_missing_input;
      }

      typename ypc::cast_obj_to_package<ntt::sealed_data_info_t>::type single =
          all_data_info[0];
      data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package(single);
    }

    if (m_ptype.d.data_source_type ==
        ypc::utc::multi_sealed_datasource_parser) {
      data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package<
          typename ypc::cast_obj_to_package<
              ntt::multi_sealed_data_info_t>::type,
          ntt::sealed_data_info_vector>(all_data_info);
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
  uint32_t feed_model() {
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
  uint32_t feed_param() { return ypc::success; }
  uint32_t dump_result(const ypc::bytes &res) {
    if (m_ptype.d.result_type == ypc::utc::onchain_result_parser) {
      auto pkg =
          ypc::make_package<ntt::onchain_result_package_t>::from_bytes(res);
      typename ypc::cast_package_to_obj<ntt::onchain_result_package_t>::type p =
          pkg;
      m_result_str = ypc::ntjson::to_json(p);
    } else if (m_ptype.d.result_type == ypc::utc::offchain_result_parser) {
      auto pkg =
          ypc::make_package<ntt::offchain_result_package_t>::from_bytes(res);
      typename ypc::cast_package_to_obj<ntt::offchain_result_package_t>::type
          p = pkg;
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
  ypc::bytes construct_access_control_policy() {
    using ntt = ypc::nt<ypc::bytes>;
    ntt::access_list_package_t alp;
    alp.set<ntt::access_list_type>(ypc::utc::access_policy_blacklist);
    alp.set<ntt::access_list>(std::vector<ntt::access_item_t>());
    return ypc::make_bytes<ypc::bytes>::for_package(alp);
  }
  void set_module(const char *lib_module, const char *mod_path) {
    m_parser =
        parser_module_loader::create_module_instance(lib_module, mod_path);
  };

protected:
  input_param_t m_param;
  ypc::utc::parser_type_t m_ptype{};

  parser_module_interface *m_parser;
  std::shared_ptr<ypc::keymgr_parser> m_keymgr_parser;
  std::unordered_map<ypc::bytes, std::shared_ptr<ypc::simple_sealed_file>>
      m_data_sources;
  std::string m_result_str;
  std::unique_ptr<char[]> m_mem_buf;
  size_t m_mem_buf_size;
};
