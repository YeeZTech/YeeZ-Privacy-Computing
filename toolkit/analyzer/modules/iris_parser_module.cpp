#include "iris_parser_module.h"
#include "iris_u.h"
#include "sgx_urts.h"
#include "ypc/corecommon/package.h"
#include <stdexcept>

iris_parser_module::iris_parser_module(const char *mod_path)
    : ypc::parser_sgx_module(mod_path) {}

extern "C" {
uint32_t ocall_get_frame(const char *ifs, uint32_t ifs_size, uint8_t **data,
                         uint32_t *len);
}

uint32_t ocall_get_frame(const char *ifs, uint32_t ifs_size, uint8_t **data,
                         uint32_t *len) {
  return 0;
}
