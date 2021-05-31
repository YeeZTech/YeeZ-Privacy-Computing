#pragma once

#include "ypc/byte.h"
#include "ypc/filesystem.h"
#include <string>

#define PKEY_FILE_NAME_LENGTH 40

std::string create_dir_if_not_exist(const std::string &base,
                                    const std::string &dir);
uint32_t write_key_pair_to_file(const std::string &filename,
                                const ypc::bytes &pkey_hex,
                                const ypc::bytes &skey_hex);

uint32_t read_key_pair_from_file(const std::string &filename,
                                 ypc::bytes &b_pkey, ypc::bytes &b_skey);

extern "C" {
uint32_t ocall_load_key_pair(const char *key_path_name, uint32_t path_size,
                             uint8_t *public_key, uint32_t pkey_size,
                             uint8_t *sealed_private_key, uint32_t sealed_size);
}
