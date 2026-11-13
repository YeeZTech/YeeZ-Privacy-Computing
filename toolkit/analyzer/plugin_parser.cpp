#include "plugin_parser.h"

#include "ypc/common/access_policy.h"
#include "ypc/core/ntjson.h"
#include "ypc/core/status.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/corecommon/package.h"
#include "ypc/stbox/stx_status.h"
#include <cstring>
#include <glog/logging.h>
#include <limits>
#include <stdexcept>

namespace {

ypc::bytes construct_access_control_policy() {
  using ntt = ypc::nt<ypc::bytes>;
  ntt::access_list_package_t package;
  package.set<ntt::access_list_type>(ypc::utc::access_policy_blacklist);
  package.set<ntt::access_list>(std::vector<ntt::access_item_t>());
  return ypc::make_bytes<ypc::bytes>::for_package(package);
}

} // namespace

plugin_parser::plugin_parser(const input_param_t &param,
                             const fid::parser_registry_selection &selection)
    : m_param(param), m_selection(selection), m_callbacks{},
      m_mem_buf_size(ypc::simple_sealed_file::blockfile_t::BlockSizeLimit) {
  m_mem_buf.reset(new char[m_mem_buf_size]);
  m_callbacks.struct_size = sizeof(m_callbacks);
  m_callbacks.host_context = this;
  m_callbacks.next_data_batch = &plugin_parser::next_data_batch_callback;
  m_callbacks.km_session_request = &plugin_parser::km_session_request_callback;
  m_callbacks.km_exchange_report = &plugin_parser::km_exchange_report_callback;
  m_callbacks.km_send_request = &plugin_parser::km_send_request_callback;
  m_callbacks.km_end_session = &plugin_parser::km_end_session_callback;
  m_callbacks.log = &plugin_parser::log_callback;
}

plugin_parser::~plugin_parser() = default;

uint32_t plugin_parser::parse() {
  const auto keymgr_enclave_path = m_param.get<keymgr_path>();
  m_keymgr =
      std::make_shared<ypc::keymgr_sgx_module>(keymgr_enclave_path.c_str());
  m_keymgr->set_access_control_policy(construct_access_control_policy());

  // Registry pairing and all file hashes were checked before this point.
  m_plugin.reset(new fid::parser_plugin(m_selection, m_callbacks));
  LOG(INFO) << "initialized parser plugin " << m_selection.module.module_id
            << " and key manager";

  const auto epkey = m_param.get<dian_pkey>();
  const auto expected_hash = m_param.get<parser_enclave_hash>();
  uint32_t ret = 0;

  const auto forward_result_info = m_param.get<forward_shu_info>();
  if (!forward_result_info.empty()) {
    if (forward_result_info.size() < 16u ||
        forward_result_info.size() > 1024u * 1024u) {
      throw std::runtime_error("invalid forward_shu_info length");
    }
    ypc::bytes forward_skey;
    ypc::bytes forward_signature;
    try {
      auto forward_package =
          ypc::make_package<typename ypc::cast_obj_to_package<
              ntt::forward_result_t>::type>::from_bytes(forward_result_info);
      const auto forward_shu = forward_package.get<ntt::shu_info>();
      forward_skey = forward_shu.get<ntt::encrypted_shu_skey>();
      forward_signature = forward_shu.get<ntt::shu_forward_signature>();
    } catch (const std::exception &error) {
      throw std::runtime_error(std::string("invalid forward_shu_info: ") +
                               error.what());
    }
    ret = m_keymgr->forward_private_key(
        forward_skey.data(), forward_skey.size(), epkey.data(), epkey.size(),
        expected_hash.data(), expected_hash.size(), forward_signature.data(),
        forward_signature.size());
    if (ret != 0u) {
      LOG(ERROR) << "forward_shu_info failed: " << ypc::status_string(ret);
      return ret;
    }
  }

  const auto normal_shu = m_param.get<shu_info>();
  const auto shu_skey = normal_shu.get<ntt::encrypted_shu_skey>();
  const auto shu_forward_signature =
      normal_shu.get<ntt::shu_forward_signature>();
  if (!shu_skey.empty()) {
    ret = m_keymgr->forward_private_key(
        shu_skey.data(), shu_skey.size(), epkey.data(), epkey.size(),
        expected_hash.data(), expected_hash.size(),
        shu_forward_signature.data(), shu_forward_signature.size());
    if (ret != 0u) {
      LOG(ERROR) << "shu_info forwarding failed: " << ypc::status_string(ret);
      return ret;
    }
  }

  uint32_t parser_type_value = 0;
  ret = m_plugin->get_parser_type(parser_type_value);
  if (ret != 0u) {
    LOG(ERROR) << "get_parser_type failed: " << ypc::status_string(ret);
    return ret;
  }
  m_parser_type.value = parser_type_value;

  ypc::bytes actual_hash;
  ret = m_plugin->get_enclave_hash(actual_hash);
  if (ret != 0u) {
    LOG(ERROR) << "get_enclave_hash failed: " << ypc::status_string(ret);
    return ret;
  }
  if (actual_hash != expected_hash) {
    LOG(ERROR) << "parser hash is " << actual_hash << ", expected "
               << expected_hash;
    return ypc::parser_return_wrong_data_hash;
  }

  ret = feed_datasource();
  if (ret != 0u) {
    return ret;
  }
  ret = feed_model();
  if (ret != 0u) {
    return ret;
  }

  ret = m_plugin->begin_parse_data_item();
  if (ret != stbox::stx_status::success) {
    LOG(ERROR) << "begin_parse_data_item failed: " << ypc::status_string(ret);
    return ret;
  }

  const auto parameter = m_param.get<ntt::param>();
  typename ypc::cast_obj_to_package<ntt::param_t>::type parameter_package =
      parameter;
  const auto parameter_bytes =
      ypc::make_bytes<ypc::bytes>::for_package(parameter_package);
  ret =
      m_plugin->parse_data_item(parameter_bytes.data(), parameter_bytes.size());
  if (ret != 0u) {
    LOG(ERROR) << "parse_data_item failed: " << ypc::status_string(ret);
    return ret;
  }

  ret = m_plugin->end_parse_data_item();
  if (ret != stbox::stx_status::success) {
    LOG(ERROR) << "end_parse_data_item failed: " << ypc::status_string(ret);
    return ret;
  }

  ypc::bytes result;
  ret = m_plugin->get_analyze_result(result);
  if (ret != 0u) {
    LOG(ERROR) << "get_analyze_result failed: " << ypc::status_string(ret);
    return ret;
  }
  return dump_result(result);
}

uint32_t plugin_parser::dump_result(const ypc::bytes &result) {
  if (m_parser_type.d.result_type == ypc::utc::onchain_result_parser) {
    auto package =
        ypc::make_package<ntt::onchain_result_package_t>::from_bytes(result);
    typename ypc::cast_package_to_obj<ntt::onchain_result_package_t>::type obj =
        package;
    m_result_str = ypc::ntjson::to_json(obj);
  } else if (m_parser_type.d.result_type == ypc::utc::offchain_result_parser) {
    auto package =
        ypc::make_package<ntt::offchain_result_package_t>::from_bytes(result);
    typename ypc::cast_package_to_obj<ntt::offchain_result_package_t>::type
        obj = package;
    m_result_str = ypc::ntjson::to_json(obj);
  } else if (m_parser_type.d.result_type == ypc::utc::local_result_parser) {
    m_result_str.assign(reinterpret_cast<const char *>(result.data()),
                        result.size());
  } else if (m_parser_type.d.result_type == ypc::utc::forward_result_parser) {
    auto package = ypc::make_package<typename ypc::cast_obj_to_package<
        ntt::forward_result_t>::type>::from_bytes(result);
    typename ypc::cast_package_to_obj<ntt::forward_result_t>::type obj =
        package;
    m_result_str = ypc::ntjson::to_json(obj);
  } else {
    return ypc::parser_unknown_result;
  }
  return ypc::success;
}

uint32_t plugin_parser::feed_datasource() {
  const auto input_items = m_param.get<input_data>();
  if (m_parser_type.d.data_source_type == ypc::utc::noinput_datasource_parser) {
    return ypc::success;
  }
  if (input_items.empty()) {
    LOG(ERROR) << "parser requires at least one input data source";
    return ypc::parser_missing_input;
  }
  if (m_parser_type.d.data_source_type ==
          ypc::utc::single_sealed_datasource_parser &&
      input_items.size() > 1) {
    LOG(WARNING) << "single-input parser ignores inputs after the first";
  }

  const auto epkey = m_param.get<dian_pkey>();
  std::vector<ntt::sealed_data_info_t> all_data_info;
  for (const auto &item : input_items) {
    const auto url = item.get<input_data_url>();
    const auto data_hash = item.get<input_data_hash>();
    const auto shu = item.get<shu_info>();
    const auto spkey = shu.get<shu_pkey>();

    auto sealed_file = std::make_shared<ypc::simple_sealed_file>(url, true);
    m_data_sources[data_hash + spkey] = sealed_file;
    sealed_file->reset_read();

    const auto shu_skey = shu.get<ntt::encrypted_shu_skey>();
    const auto shu_signature = shu.get<ntt::shu_forward_signature>();
    const auto target_hash = shu.get<enclave_hash>();
    const uint32_t ret = m_keymgr->forward_private_key(
        shu_skey.data(), shu_skey.size(), epkey.data(), epkey.size(),
        target_hash.data(), target_hash.size(), shu_signature.data(),
        shu_signature.size());
    if (ret != 0u) {
      LOG(ERROR) << "data source key forwarding failed: "
                 << ypc::status_string(ret);
      return ret;
    }

    ntt::sealed_data_info_t info;
    info.set<ntt::data_hash, ntt::pkey, ntt::tag>(data_hash, spkey,
                                                  item.get<ntt::tag>());
    all_data_info.push_back(info.make_copy());
  }

  ypc::bytes data_info_bytes;
  if (m_parser_type.d.data_source_type ==
      ypc::utc::single_sealed_datasource_parser) {
    typename ypc::cast_obj_to_package<ntt::sealed_data_info_t>::type single =
        all_data_info.front();
    data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package(single);
  } else if (m_parser_type.d.data_source_type ==
             ypc::utc::multi_sealed_datasource_parser) {
    data_info_bytes = ypc::make_bytes<ypc::bytes>::for_package<
        typename ypc::cast_obj_to_package<ntt::multi_sealed_data_info_t>::type,
        ntt::sealed_data_info_vector>(all_data_info);
  } else if (m_parser_type.d.data_source_type ==
             ypc::utc::raw_datasource_parser) {
    data_info_bytes = all_data_info.front().get<ntt::data_hash>();
  }

  const uint32_t ret = m_plugin->init_data_source(
      data_info_bytes.data(), static_cast<uint32_t>(data_info_bytes.size()));
  if (ret != 0u) {
    LOG(ERROR) << "init_data_source failed: " << ypc::status_string(ret);
  }
  return ret;
}

uint32_t plugin_parser::feed_model() {
  if (m_parser_type.d.has_model == ypc::utc::no_model_parser) {
    return ypc::success;
  }
  const auto model = m_param.get<ntt::model>();
  ypc::cast_obj_to_package<ntt::model_t>::type package = model;
  const auto bytes = ypc::make_bytes<ypc::bytes>::for_package(package);
  const uint32_t ret =
      m_plugin->init_model(bytes.data(), static_cast<uint32_t>(bytes.size()));
  if (ret != 0u) {
    LOG(ERROR) << "init_model failed: " << ypc::status_string(ret);
  }
  return ret;
}

uint32_t plugin_parser::next_data_batch(const uint8_t *hash_and_pkey,
                                        uint32_t size, uint8_t **data,
                                        uint32_t *len) {
  if (data == nullptr || len == nullptr) {
    return stbox::stx_status::invalid_parameter;
  }
  *data = nullptr;
  *len = 0;
  if (hash_and_pkey == nullptr || size < 32) {
    return stbox::stx_status::invalid_parameter;
  }
  const ypc::bytes key(hash_and_pkey, size);
  const auto found = m_data_sources.find(key);
  if (found == m_data_sources.end()) {
    LOG(ERROR) << "requested data source was not registered";
    return stbox::stx_status::data_source_not_found;
  }

  size_t result_size = 0;
  const bool success =
      found->second->next_item(m_mem_buf.get(), m_mem_buf_size, result_size) ==
      ypc::simple_sealed_file::blockfile_t::succ;
  if (success && result_size != 0) {
    *data = reinterpret_cast<uint8_t *>(m_mem_buf.get());
    *len = static_cast<uint32_t>(result_size);
    return stbox::stx_status::success;
  }
  return stbox::stx_status::sealed_file_reach_end;
}

uint32_t plugin_parser::next_data_batch_callback(void *context,
                                                 const uint8_t *hash_and_pkey,
                                                 uint32_t size, uint8_t **data,
                                                 uint32_t *len) {
  if (data != nullptr) {
    *data = nullptr;
  }
  if (len != nullptr) {
    *len = 0;
  }
  try {
    if (context == nullptr) {
      return stbox::stx_status::invalid_parameter;
    }
    return static_cast<plugin_parser *>(context)->next_data_batch(
        hash_and_pkey, size, data, len);
  } catch (...) {
    if (data != nullptr) {
      *data = nullptr;
    }
    if (len != nullptr) {
      *len = 0;
    }
    return stbox::stx_status::error_unexpected;
  }
}

uint32_t plugin_parser::km_session_request_callback(void *context,
                                                    sgx_dh_msg1_t *dh_msg1,
                                                    uint32_t *session_id) {
  if (dh_msg1 != nullptr) {
    std::memset(dh_msg1, 0, sizeof(*dh_msg1));
  }
  if (session_id != nullptr) {
    *session_id = 0;
  }
  try {
    if (context == nullptr || dh_msg1 == nullptr || session_id == nullptr) {
      return stbox::stx_status::invalid_parameter;
    }
    return static_cast<plugin_parser *>(context)->m_keymgr->session_request(
        dh_msg1, session_id);
  } catch (...) {
    if (dh_msg1 != nullptr) {
      std::memset(dh_msg1, 0, sizeof(*dh_msg1));
    }
    if (session_id != nullptr) {
      *session_id = 0;
    }
    return stbox::stx_status::error_unexpected;
  }
}

uint32_t plugin_parser::km_exchange_report_callback(void *context,
                                                    sgx_dh_msg2_t *dh_msg2,
                                                    sgx_dh_msg3_t *dh_msg3,
                                                    uint32_t session_id) {
  if (dh_msg3 != nullptr) {
    std::memset(dh_msg3, 0, sizeof(*dh_msg3));
  }
  try {
    if (context == nullptr || dh_msg2 == nullptr || dh_msg3 == nullptr) {
      return stbox::stx_status::invalid_parameter;
    }
    return static_cast<plugin_parser *>(context)->m_keymgr->exchange_report(
        dh_msg2, dh_msg3, session_id);
  } catch (...) {
    if (dh_msg3 != nullptr) {
      std::memset(dh_msg3, 0, sizeof(*dh_msg3));
    }
    return stbox::stx_status::error_unexpected;
  }
}

uint32_t plugin_parser::km_send_request_callback(
    void *context, uint32_t session_id, secure_message_t *request,
    uint64_t request_size, uint64_t max_payload_size,
    secure_message_t *response, uint64_t response_size) {
  const bool response_size_valid =
      response_size <= std::numeric_limits<size_t>::max();
  if (response != nullptr && response_size_valid) {
    std::memset(response, 0, static_cast<size_t>(response_size));
  }
  try {
    if (context == nullptr || request == nullptr || response == nullptr ||
        request_size > std::numeric_limits<size_t>::max() ||
        max_payload_size > std::numeric_limits<size_t>::max() ||
        !response_size_valid) {
      return stbox::stx_status::invalid_parameter;
    }
    return static_cast<plugin_parser *>(context)->m_keymgr->generate_response(
        request, static_cast<size_t>(request_size),
        static_cast<size_t>(max_payload_size), response,
        static_cast<size_t>(response_size), session_id);
  } catch (...) {
    if (response != nullptr && response_size_valid) {
      std::memset(response, 0, static_cast<size_t>(response_size));
    }
    return stbox::stx_status::error_unexpected;
  }
}

uint32_t plugin_parser::km_end_session_callback(void *context,
                                                uint32_t session_id) {
  try {
    if (context == nullptr) {
      return stbox::stx_status::invalid_parameter;
    }
    return static_cast<plugin_parser *>(context)->m_keymgr->end_session(
        session_id);
  } catch (...) {
    return stbox::stx_status::error_unexpected;
  }
}

void plugin_parser::log_callback(void *, uint32_t rank, const char *message,
                                 uint32_t message_size) {
  try {
    const std::string text =
        message == nullptr ? std::string() : std::string(message, message_size);
    if (rank >= 2) {
      LOG(ERROR) << "[parser plugin] " << text;
    } else {
      LOG(INFO) << "[parser plugin] " << text;
    }
  } catch (...) {
  }
}
