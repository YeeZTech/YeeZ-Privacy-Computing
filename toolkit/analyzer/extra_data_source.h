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

extern "C" {
uint32_t ocall_read_next_extra_data_item(uint8_t *data_hash,
                                         uint32_t hash_size);
uint32_t ocall_get_next_extra_data_item_size();
uint32_t ocall_get_next_extra_data_item_data(uint8_t *item_data,
                                             uint32_t ndi_size);
}

class extra_data_source_reader;
extern std::shared_ptr<extra_data_source_reader> g_data_source_reader;
