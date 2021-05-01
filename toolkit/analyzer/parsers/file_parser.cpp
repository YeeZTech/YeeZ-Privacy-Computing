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
  m_sf = std::shared_ptr<ypc::internal::sealed_file_base>(
      new ypc::simple_sealed_file(m_sealfile_path, true));
  m_sf->reset_read();

  auto ret = m_parser->begin_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "got error: " << std::to_string(ret);
    return ret;
  }
  ret = m_parser->parse_data_item((const char *)m_psource->input().data(),
                                  m_psource->input().size());
  if (ret) {
    return ret;
  }

  ret = m_parser->end_parse_data_item();
  if (ret != stx_status::success) {
    LOG(ERROR) << "got error: " << std::to_string(ret);
  }
  return ret;
}

uint32_t file_parser::next_sealed_item_data(uint8_t **data, uint32_t *len) {
  ypc::memref item_data;
  m_sf->next_item(item_data);
  if (item_data.len() == 0) {
    LOG(ERROR) << "cannot read any data";
    return static_cast<uint32_t>(stx_status::error_unexpected);
  }
  *data = reinterpret_cast<uint8_t *>(item_data.data());
  *len = item_data.len();
  return static_cast<uint32_t>(stx_status::success);
}
void file_parser::free_sealed_item_data(uint8_t *data) { delete[] data; }

