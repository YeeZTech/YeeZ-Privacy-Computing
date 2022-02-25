#pragma once
#include <string>
#include <ypc/byte.h>

uint32_t check_sealed_data(const std::string &sealer_path,
                           const std::string &sealed_file_path,
                           const ypc::bytes &data_hash);
