#include "extra_data_source_reader.h"
#include "stbox/stx_status.h"

extra_data_source_reader::extra_data_source_reader(
    const ypc::extra_data_source_t &data_source)
    : m_data_source(data_source) {}

uint32_t
extra_data_source_reader::next_extra_data_item(const uint8_t *data_hash,
    uint32_t hash_size) {
  if(hash_size != m_current_data_hash.size() ||
      memcmp(m_current_data_hash.data(), data_hash, hash_size) != 0){
    std::string fp = find_file_path_for_data(data_hash, hash_size);
    if (fp.empty()) {
      return stbox::stx_status::extra_data_file_path_not_found;
    }
    m_current_file.reset(new ypc::simple_sealed_file(fp, true));
    m_current_data_hash = ypc::bytes(data_hash, hash_size);
  }

  m_has_value = m_current_file->next_item(m_data);
  if (!m_has_value) {
    return stbox::stx_status::extra_data_file_reach_end;
  }
  return 0;
}

uint32_t extra_data_source_reader::get_next_extra_data_item_size(){
  if (!m_has_value) {
    return 0;
  }
  return m_data.size();
}
uint32_t extra_data_source_reader::get_next_extra_data_item_data(uint8_t * item_data, uint32_t ndi_size){
  if (!m_has_value) {
    return 0;
  }
  memcpy(item_data, m_data.data(), ndi_size);
  return 0;
}

std::string
extra_data_source_reader::find_file_path_for_data(const uint8_t *data_hash,
                                                  uint32_t hash_size) {
  for(auto group : m_data_source){
    for (auto data_item : group.get<ypc::extra_data_set>()) {
      const ypc::bytes &hash = data_item.get<::ypc::data_hash>();
      if (memcmp(hash.data(), data_hash, hash.size()) == 0) {
        return data_item.get<ypc::file_path>();
      }
    }
  }
  return std::string();
}
