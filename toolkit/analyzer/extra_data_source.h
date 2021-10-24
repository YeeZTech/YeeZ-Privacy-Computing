#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <vector>
#include <ypc/byte.h>

namespace ypc {
define_nt(file_path, std::string);
define_nt(data_use_license, ypc::bytes);
define_nt(data_hash, ypc::bytes);
typedef ::ff::util::ntobject<file_path, data_use_license, data_hash>
    extra_data_item_t;

define_nt(extra_data_set, std::vector<extra_data_item_t>);
define_nt(extra_data_group_name, std::string);
typedef ::ff::util::ntobject<extra_data_group_name, extra_data_set>
    extra_data_group_t;

typedef std::vector<extra_data_group_t> extra_data_source_t;

extra_data_source_t
read_extra_data_source_from_file(const std::string &file_path);

void write_extra_data_source_to_file(const extra_data_source_t _data_source,
                                     const std::string &file_path);

} // namespace ypc
