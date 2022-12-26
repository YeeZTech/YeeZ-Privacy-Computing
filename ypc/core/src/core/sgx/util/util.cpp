#include "ypc/core/sgx/util/util.h"
#include "ypc/core/sgx/util/cxxfile_bridge.h"
#include "ypc/core/sgx/util/kv_bridge.h"
#include <string>

namespace ypc {
std::string g_sgx_file_directory("/tmp/");

void set_sgx_file_dir(const char *directory) {
  g_sgx_file_directory = std::string(directory);
}

const char *get_sgx_file_dir(const char *directory) {
  return g_sgx_file_directory.c_str();
}

void init_sgx_env() {
  init_sgx_cxxfile();
  init_sgx_kv();
}
void shutdown_sgx_env() {
  shutdown_sgx_cxxfile();
  shutdown_sgx_kv();
}
} // namespace ypc
