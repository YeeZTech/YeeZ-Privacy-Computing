#include "personlist_parser_module.h"
#include "person_u.h"

personlist_parser_module::personlist_parser_module(const char *mod_path)
    : ypc::parser_sgx_module(mod_path) {}


extern "C" {
uint32_t ocall_get_personlist(const char *ifs, uint32_t ifs_size,
                              uint8_t **data, uint32_t *len);
}

uint32_t ocall_get_personlist(const char *ifs, uint32_t ifs_size,
                              uint8_t **data, uint32_t *len) {
  LOG(INFO) << "ocall get personlist";
  return 0;
}
extern "C" ypc::parser_sgx_module *create_instance(std::string mod_path) {
  return new personlist_parser_module(mod_path.c_str());
}
