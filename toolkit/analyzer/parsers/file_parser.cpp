#include "parsers/file_parser.h"

file_parser::file_parser(param_source *psource, result_target *rtarget,
                         const std::string &sealer_enclave_path,
                         const std::string &parser_enclave_path,
                         const std::string &keymgr_enclave_path,
                         const std::string &sealfile_path)
    : parser_base(psource, rtarget, sealer_enclave_path, parser_enclave_path,
                  keymgr_enclave_path),
      m_sealfile_path(sealfile_path) {}

uint32_t file_parser::do_parse() {
  // m_sf = std::shared_ptr<ypc::internal::sealed_file_base>(
  // new ypc::sealed_file_with_cache_opt(m_sealfile_path, true));
  try {
    m_sf = std::shared_ptr<ypc::internal::sealed_file_base>(
        new ypc::simple_sealed_file(m_sealfile_path, true));
    m_sf->reset_read();
  } catch (const std::exception &e) {
    LOG(ERROR) << "error while open file " << m_sealfile_path;
    return ypc::sealed_file_cannot_open;
  }

  auto ret = m_parser->begin_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "begin_parse_data_item, got error: "
               << ypc::status_string(ret);
    return ret;
  }
  ret = m_parser->parse_data_item((const char *)m_psource->input().data(),
                                  m_psource->input().size());
  if (ret) {
    LOG(ERROR) << "parse_data_item, got error: " << ypc::status_string(ret);
    return ret;
  }

  ret = m_parser->end_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "end_parse_data_item, got error: " << ypc::status_string(ret);
  }
  return ret;
}

uint32_t file_parser::next_sealed_item_data(uint8_t **data, uint32_t *len) {
  ypc::memref item_data;
  m_sf->next_item(item_data);
  if (item_data.size() == 0) {
    LOG(WARNING) << "read data reach end ";
    return static_cast<uint32_t>(stx_status::sealed_file_reach_end);
  }
  *data = reinterpret_cast<uint8_t *>(item_data.data());
  *len = item_data.size();
  return static_cast<uint32_t>(stx_status::success);
}
void file_parser::free_sealed_item_data(uint8_t *data) { delete[] data; }

