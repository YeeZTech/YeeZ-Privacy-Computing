#pragma once

namespace ypc {
void set_sgx_file_dir(const char *directory);

const char *get_sgx_file_dir(const char *directory);

void init_sgx_env();
void shutdown_sgx_env();
}
