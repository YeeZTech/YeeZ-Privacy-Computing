#include "parsers/parser_base.h"
#include "ypc/param_id.h"
#include "ypc/sealed_file.h"
#include <glog/logging.h>

parser_base::parser_base(param_source *psource, result_target *rtarget,
                         const std::string &sealer_enclave_path,
                         const std::string &parser_enclave_path,
                         const std::string &keymgr_enclave_path)
    : m_psource(psource), m_rtarget(rtarget),
      m_sealer_enclave_path(sealer_enclave_path),
      m_parser_enclave_path(parser_enclave_path),
      m_keymgr_enclave_path(keymgr_enclave_path) {}

parser_base::~parser_base() {}

void parser_base::parse() {
  m_sealer =
      std::make_shared<ypc::datahub_sgx_module>(m_sealer_enclave_path.c_str());
  m_parser = std::make_shared<parser_sgx_module>(m_parser_enclave_path.c_str());
  m_keymgr = std::make_shared<keymgr_sgx_module>(m_keymgr_enclave_path.c_str());
  LOG(INFO) << "initializing datahub/parser/keymgr module done";

  m_psource->read_from_source();
  LOG(INFO) << "read data source done";

  auto eskey = m_psource->eskey();
  auto epkey = m_psource->epkey();
  auto ehash = m_psource->ehash();
  auto vpkey = m_psource->vpkey();
  auto sig = m_psource->sig();
  m_keymgr->forward_message(param_id::PRIVATE_KEY, eskey.value(), eskey.size(),
                            epkey.value(), epkey.size(), ehash.value(),
                            ehash.size(), vpkey.value(), vpkey.size(),
                            sig.value(), sig.size());
  LOG(INFO) << "forward private key done";
  do_parse();
  LOG(INFO) << "parse done";

  ypc::bref encrypted_res, result_sig, data_hash;
  m_parser->get_encrypted_result_and_signature(encrypted_res, result_sig);
  m_parser->get_data_hash(data_hash);

  m_rtarget->write_to_target(encrypted_res, result_sig, data_hash);
  LOG(INFO) << "write result target done";
}

bool parser_base::merge(
    std::vector<std::shared_ptr<result_target>> &block_results) {
  if (!m_parser) {
    m_sealer = std::make_shared<ypc::datahub_sgx_module>(
        m_sealer_enclave_path.c_str());
    m_parser =
        std::make_shared<parser_sgx_module>(m_parser_enclave_path.c_str());

    m_keymgr =
        std::make_shared<keymgr_sgx_module>(m_keymgr_enclave_path.c_str());
    LOG(INFO) << "initializing datahub/parser/keymgr module done";

    m_psource->read_from_source();

    auto eskey = m_psource->eskey();
    auto epkey = m_psource->epkey();
    auto ehash = m_psource->ehash();
    auto vpkey = m_psource->vpkey();
    auto sig = m_psource->sig();
    m_keymgr->forward_message(param_id::PRIVATE_KEY, eskey.value(),
                              eskey.size(), epkey.value(), epkey.size(),
                              ehash.value(), ehash.size(), vpkey.value(),
                              vpkey.size(), sig.value(), sig.size());
    LOG(INFO) << "keymgr forward_message done!";
  }

  for (uint16_t i = 0; i < block_results.size(); ++i) {
    ypc::bytes encrypted_result;
    ypc::bytes sig;
    ypc::bytes hash;
    block_results[i]->read_from_target(encrypted_result, sig, hash);
    m_parser->add_block_parse_result(i, encrypted_result, hash, sig);
  }
  LOG(INFO) << "add_block_parse_result done";

  auto ret = m_parser->begin_parse_data_item();
  if (ret != stx_status::success) {
    LOG(INFO) << "got error: " << std::to_string(ret);
  }

  m_parser->merge_parse_result(m_psource->input());
  LOG(INFO) << "merge_parse_result done";

  ret = m_parser->end_parse_data_item();
  if (ret != stx_status::success) {
    LOG(INFO) << "got error: " << std::to_string(ret);
  }

  ypc::bref encrypted_res, result_sig, data_hash;
  m_parser->get_encrypted_result_and_signature(encrypted_res, result_sig);
  m_parser->get_data_hash(data_hash);
  LOG(INFO) << "get_encrypted_result done";

  m_rtarget->write_to_target(encrypted_res, result_sig, data_hash);
  LOG(INFO) << "write result target done";
  return m_parser->need_continue();
}

