#pragma once
#include "ypc_t/analyzer/helper/parser_type_traits.h"
#include "ypc_t/analyzer/interface/algo_interface.h"
#include "ypc_t/analyzer/interface/data_hash_interface.h"
#include "ypc_t/analyzer/interface/data_interface.h"
#include "ypc_t/analyzer/interface/model_interface.h"
#include "ypc_t/analyzer/interface/parser_interface.h"

#include "ypc_t/analyzer/internal/keymgr_session.h"
#include "ypc_t/analyzer/internal/local_result.h"
#include "ypc_t/analyzer/internal/multi_data_stream.h"
#include "ypc_t/analyzer/internal/noinput_data_stream.h"
#include "ypc_t/analyzer/internal/offchain_result.h"
#include "ypc_t/analyzer/internal/onchain_result.h"
#include "ypc_t/analyzer/internal/raw_data_stream.h"
#include "ypc_t/analyzer/internal/sealed_data_stream.h"

namespace ypc {
template <typename Crypto, typename DataSession, typename ParserT,
          typename Result, typename ModelT = void>
class algo_wrapper
    : virtual public internal::algo_interface<Crypto, ParserT, Result, ModelT>,
      virtual public Result,
      virtual public internal::keymgr_session,
      virtual public internal::parser_interface<DataSession, ParserT>,
      virtual public internal::data_interface<Crypto, DataSession>,
      virtual public internal::data_hash_interface<DataSession>,
      virtual public internal::model_interface<Crypto, ModelT> {
  typedef internal::keymgr_session keymgr_session_t;
  typedef internal::parser_interface<DataSession, ParserT> parser_interface_t;
  typedef internal::algo_interface<Crypto, ParserT, Result, ModelT>
      algo_interface_t;
  typedef internal::data_hash_interface<DataSession> data_hash_interface_t;

public:
  uint32_t begin_parse_data_item() {
    LOG(INFO) << "start init keymgr session...";
    uint32_t ret = keymgr_session_t::init_keymgr_session();
    if (ret) {
      LOG(ERROR) << "parse_data_item_impl failed: "
                 << stbox::status_string(ret);
      return ret;
    }
    LOG(INFO) << "init keymgr session done, start create_parser";
    ret = parser_interface_t::create_parser();
    if (ret) {
      LOG(ERROR) << "create_parser failed: " << stbox::status_string(ret);
      return ret;
    }
    LOG(INFO) << "begin_parse_data_item done";
    return stbox::stx_status::success;
  }

  uint32_t parse_data_item(const uint8_t *input_param, uint32_t len) {
    uint32_t ret = algo_interface_t::parse_data_item_impl(input_param, len);
    if (ret) {
      LOG(ERROR) << "parse_data_item_impl failed: "
                 << stbox::status_string(ret);
      return ret;
    }
    data_hash_interface_t::set_data_hash();
    return stbox::stx_status::success;
  }

  uint32_t end_parse_data_item() {
    uint32_t ret = Result::generate_result();
    if (ret) {
      LOG(ERROR) << "generate result failed: " << stbox::status_string(ret);
      return ret;
    }
    return keymgr_session_t::close_keymgr_session();
  }

  uint32_t get_parser_type() const {
    ypc::utc::parser_type_t type;
    type.d.result_type = result_type_traits<Result>::value;
    type.d.data_source_type = datasource_type_traits<DataSession>::value;
    type.d.has_model = model_type_traits<ModelT>::value;
    return type.value;
  }
};

// template <typename ParserT>
// using onchain_classical =
// algo_wrapper<sealed_data_stream, ParserT, onchain_result>;

// template <typename ParserT>
// using offchain_classical =
// algo_wrapper<sealed_data_stream, ParserT, offchain_result>;

// template <typename ParserT>
// using multi_hosting_data_onchain =
// algo_wrapper<multi_data_stream, ParserT, onchain_result>;

// template <typename ParserT>
// using multi_hosting_data_offchain =
// algo_wrapper<multi_data_stream, ParserT, offchain_result>;

//// template <typename ParserT>
//// using multi_dist_data_onchain = algo_wrapper
// template <typename ParserT, typename ModelT>
// using local_parser =
// algo_wrapper<raw_data_stream, ParserT, local_result, ModelT>;

// template <typename ParserT, typename ModelT>
// using local_parser =
// algo_wrapper<raw_data_stream, ParserT, offchain_result, ModelT>;
} // namespace ypc
