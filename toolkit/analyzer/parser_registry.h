#pragma once

#include "ypc/core/byte.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace fid {

struct parser_module_record {
  std::string module_id;
  uint32_t abi_major;
  uint32_t abi_minor;
  std::string edl_fingerprint;
  std::string library_path;
  std::string library_sha256;
  std::string fragment_path;
};

struct parser_enclave_record {
  std::string module_id;
  std::string edl_fingerprint;
  std::string enclave_sha256;
  std::string mrenclave;
  std::string fragment_path;
};

struct parser_registry_selection {
  parser_module_record module;
  parser_enclave_record enclave;
  std::string enclave_path;
};

class parser_registry {
public:
  void load(const std::vector<std::string> &directories);

  parser_registry_selection select(const std::string &module_id,
                                   const std::string &enclave_path,
                                   const ypc::bytes &expected_mrenclave) const;

  static std::string sha256_file(const std::string &path);
  static std::string sha256_fd(int descriptor);
  static void require_trusted_fd(int descriptor, const char *description,
                                 bool allow_effective_group_write = false);
  static void require_trusted_file(const std::string &path,
                                   const char *description,
                                   bool allow_effective_group_write = false);

private:
  void load_fragment(const std::string &path);

  std::map<std::string, parser_module_record> m_modules;
  std::map<std::string, parser_enclave_record> m_enclaves_by_sha256;
};

std::string normalize_hex(std::string value);
bool valid_module_id(const std::string &value);

} // namespace fid
