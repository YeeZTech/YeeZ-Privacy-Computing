#pragma once
#include "iodef.h"
#include "ypc/common/parser_type.h"
#include "ypc/core/sealed_file.h"
#include "ypc/keymgr/default/keymgr_sgx_module.h"
#include "ypc/keymgr/default/keymgr_bridge.h"
#include "ypc/core/sgx/parser_sgx_module.h"
#include <dlfcn.h>
#include <memory>
#include <unordered_map>

class dianshu_parser {
public:
  using CreateInstanceFunc = ypc::parser_sgx_module *(*)(std::string);
  static dianshu_parser* GetParser();
  virtual ~dianshu_parser() = default;

  void Init(const input_param_t &param, const std::string &lib_module_path);

  virtual uint32_t parse();

  virtual uint32_t next_data_batch(const uint8_t *hash_and_pkey,
                                   uint32_t hash_and_pkey_size, uint8_t **data,
                                   uint32_t *len);

  inline std::shared_ptr<ypc::keymgr_sgx_module> keymgr() const {
    return m_keymgr_parser->keymgr();
  }
  inline const std::string &get_result_str() const { return m_result_str; }

protected:
  dianshu_parser() = default;
  ypc::bytes construct_access_control_policy();
  uint32_t feed_datasource();
  uint32_t feed_model();
  uint32_t feed_param();
  uint32_t dump_result(const ypc::bytes &res);

protected:
  template <typename T> T get_func_with_name(const std::string &name) {
    T r = (T)dlsym(m_lib_module, name.c_str());
    if (!r) {
      throw std::runtime_error(m_lib_module_path + "::" + name +
                               "::" + dlerror());
    }
    return r;
  }

  // NOLINTBEGIN(modernize-use-using)
  // typedef void *(*create_parser_module_func_t)(const char *mod_path);  // NOLINT


protected:
  input_param_t m_param;
  ypc::utc::parser_type_t m_ptype{};

  std::shared_ptr<ypc::parser_sgx_module> m_parser;
  std::shared_ptr<ypc::keymgr_parser> m_keymgr_parser;
  std::unordered_map<ypc::bytes, std::shared_ptr<ypc::simple_sealed_file>>
      m_data_sources;
  std::string m_result_str;
  std::unique_ptr<char[]> m_mem_buf;
  size_t m_mem_buf_size;

  std::string m_lib_module_path;
  void *m_lib_module;
  CreateInstanceFunc m_create_parser_module;
  // std::unordered_map<std::string, CreateInstanceFunc> parserMap;
};
