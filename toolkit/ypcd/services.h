
#pragma once
#include "common/package.h"
#include "db.h"
#include <string>

namespace toolkit {
namespace ypcd {
void register_data_meta_service(register_data_meta_pkg_t *pkg_ptr,
                                ::ff::sql::mysql<::ff::sql::cppconn> *db);

void start_data_analysis_service();
}
} // namespace toolkit
