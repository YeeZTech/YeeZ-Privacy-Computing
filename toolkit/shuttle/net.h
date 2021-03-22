#pragma once
#include "common/package.h"
#include <ff/network.h>

namespace toolkit {
namespace shuttle {

void send_data_meta(const std::string &data_id, const std::string &data_name,
                    const std::string &exec_parser_path,
                    const std::string &exec_parser_param,
                    const std::string &sealed_data_path);
}
} // namespace toolkit
