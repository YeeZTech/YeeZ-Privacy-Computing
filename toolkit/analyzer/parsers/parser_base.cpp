#include "parsers/parser_base.h"
#include "common/param_id.h"
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

uint32_t parser_base::parse() {
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
  auto ret = m_keymgr->forward_message(param_id::PRIVATE_KEY, eskey.data(),
                                       eskey.size(), epkey.data(), epkey.size(),
                                       ehash.data(), ehash.size(), vpkey.data(),
                                       vpkey.size(), sig.data(), sig.size());
  if (ret) {
    return ret;
  }
  LOG(INFO) << "forward private key done";
  ret = do_parse();

  if (ret) {
    return ret;
  }
  LOG(INFO) << "parse done";

  ypc::bref encrypted_res, result_sig, data_hash, cost_sig;
  ret = m_parser->get_encrypted_result_and_signature(encrypted_res, result_sig,
                                                     cost_sig);
  if (ret) {
    return ret;
  }
  ret = m_parser->get_data_hash(data_hash);
  if (ret) {
    return ret;
  }

  m_rtarget->write_to_target(encrypted_res, result_sig, cost_sig, data_hash);
  LOG(INFO) << "write result target done";
  return ypc::success;
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

    uint32_t ret = m_psource->read_from_source();
    if (ret) {
      return ret;
    }

    auto eskey = m_psource->eskey();
    auto epkey = m_psource->epkey();
    auto ehash = m_psource->ehash();
    auto vpkey = m_psource->vpkey();
    auto sig = m_psource->sig();
    ret = m_keymgr->forward_message(param_id::PRIVATE_KEY, eskey.data(),
                                    eskey.size(), epkey.data(), epkey.size(),
                                    ehash.data(), ehash.size(), vpkey.data(),
                                    vpkey.size(), sig.data(), sig.size());
    if (ret) {
      return ret;
    }
    LOG(INFO) << "keymgr forward_message done!";
  }

  for (uint16_t i = 0; i < block_results.size(); ++i) {
    ypc::bytes encrypted_result;
    ypc::bytes sig;
    ypc::bytes hash;
    ypc::bytes cost_sig;
    block_results[i]->read_from_target(encrypted_result, sig, cost_sig, hash);
    // TODO seems we missed the batched cost here
    m_parser->add_block_parse_result(i, encrypted_result, hash, sig);
  }
  LOG(INFO) << "add_block_parse_result done";

  auto ret = m_parser->begin_parse_data_item();
  if (ret != stx_status::success) {
    LOG(INFO) << "got error: " << std::to_string(ret);
    return ret;
  }

  ret = m_parser->merge_parse_result(m_psource->input());
  if (ret) {
    return ret;
  }
  LOG(INFO) << "merge_parse_result done";

  ret = m_parser->end_parse_data_item();
  if (ret != stx_status::success) {
    LOG(INFO) << "got error: " << std::to_string(ret);
    return ret;
  }

  ypc::bref encrypted_res, result_sig, data_hash, cost_sig;
  ret = m_parser->get_encrypted_result_and_signature(encrypted_res, result_sig,
                                                     cost_sig);
  if (ret) {
    return ret;
  }
  ret = m_parser->get_data_hash(data_hash);
  if (ret) {
    return ret;
  }
  LOG(INFO) << "get_encrypted_result done";

  m_rtarget->write_to_target(encrypted_res, result_sig, cost_sig, data_hash);
  LOG(INFO) << "write result target done";
  return m_parser->need_continue();
}

