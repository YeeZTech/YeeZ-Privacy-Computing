#pragma once
#include "common/package.h"
#include "db.h"
#include <ff/network.h>

namespace toolkit {
namespace ypcd {

void start_net_service(const std::string &conf_file,
                       ::ff::sql::mysql<::ff::sql::cppconn> *db);

void send_exit_code(const std::string &conf_file);
} // namespace ypcd
} // namespace toolkit
