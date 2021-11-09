#include "parsers/parser_base.h"
#include "common/param_id.h"
#include "corecommon/nt_cols.h"
#include "corecommon/package.h"
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

  forward_extra_data_usage_license(epkey);

  if (ret) {
    return ret;
  }
  LOG(INFO) << "forward private key done";
  ret = do_parse();

  if (ret) {
    return ret;
  }
  LOG(INFO) << "parse done";

  result_pkg_t result_pkg;
  ret = m_parser->get_analyze_result(result_pkg);
  if (ret) {
    return ret;
  }

  // TODO, check data hash

  m_rtarget->write_to_target(result_pkg);
  LOG(INFO) << "write result target done";
  return ypc::success;
}

void parser_base::forward_extra_data_usage_license(
    const ypc::bytes &enclave_pkey) {
  typedef ypc::nt<ypc::bytes> ntt;

  std::vector<ntt::extra_data_group_t> data_items;
  LOG(INFO) << "extra data source group size: " << m_extra_data_source.size();
  for (auto edg : m_extra_data_source) {
    ntt::extra_data_group_t group;
    group.set<ntt::extra_data_group_name>(
        edg.get<ypc::extra_data_group_name>());
    std::vector<ypc::bytes> hashes;
    LOG(INFO) << "extra data item size in group "
              << edg.get<ypc::extra_data_group_name>() << ", "
              << edg.get<ypc::extra_data_set>().size();
    for (auto ed_item : edg.get<ypc::extra_data_set>()) {
      ypc::bytes data_hash = ed_item.get<ypc::data_hash>();
      ypc::bytes data_use_license = ed_item.get<ypc::data_use_license>();
      m_keymgr->forward_extra_data_usage_license(enclave_pkey, data_hash,
                                                 data_use_license);
      hashes.push_back(data_hash);
    }
    group.set<ntt::extra_data_hashes>(hashes);
    data_items.push_back(group);
  }
  ypc::bytes extra = ypc::make_bytes<ypc::bytes>::for_package<
      ntt::extra_data_package_t, ntt::extra_data_items>(data_items);
  m_parser->set_extra_data(extra.data(), extra.size());
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

  result_pkg_t result_pkg;
  ret = m_parser->get_analyze_result(result_pkg);
  if (ret) {
    return ret;
  }

  // TODO, check data hash

  m_rtarget->write_to_target(result_pkg);
  LOG(INFO) << "write result target done";
  return m_parser->need_continue();
}

