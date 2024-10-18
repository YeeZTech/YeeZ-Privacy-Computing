#include "person_parser_module.h"
#include "person_u.h"
#include "sgx_urts.h"
#include "ypc/corecommon/package.h"
#include <stdexcept>

person_parser_module::person_parser_module(const char *mod_path)
    : ypc::parser_sgx_module(mod_path) {}

extern "C" {
uint32_t ocall_get_page(const char *ifs, uint32_t ifs_size, uint8_t **data,
                         uint32_t *len);
}

uint32_t ocall_get_page(const char *ifs, uint32_t ifs_size, uint8_t **data,
                         uint32_t *len) {
  LOG(INFO) << "ocall_get_page";                   
  return 0;
}
