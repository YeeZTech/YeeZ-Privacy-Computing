#include "services.h"
#include "db.h"
#include "ypc/configuration.h"
#include "ypc/filesystem.h"
#include "ypc/timer_loop.h"
#include <chrono>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace toolkit {
namespace ypcd {

void register_data_meta_service(register_data_meta_pkg_t *pkg_ptr,
                                ::ff::sql::mysql<::ff::sql::cppconn> *db) {
  auto data_hash = pkg_ptr->get<data_id_c>();
  auto data_name = pkg_ptr->get<data_desc_c>();
  auto now = std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  auto sealed_data_path = pkg_ptr->get<sealed_data_path_c>();
  privacy_data_item_t item;
  item.set<data_id, s_name, ts, extra_path>(data_hash, data_name, now,
                                            sealed_data_path);
  privacy_data_table::row_collection_type rs;
  rs.push_back(item);
  privacy_data_table::insert_or_replace_rows(db, rs);
}

void start_data_analysis_service() {}
} // namespace ypcd
} // namespace toolkit
