#include "../project_path.h"
#include "ypc/common/parser_type.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/sgx/parser_sgx_module.h"
#include "ypc/keymgr/default/keymgr_sgx_module.h"
#include <memory>
#include <unordered_map>

#include "./gtest_common.h"
#include "ypc/core/blockfile.h"
#include "ypc/core/status.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/corecommon/package.h"
#include <gtest/gtest.h>

using ntt = ypc::nt<ypc::bytes>;

class parser {
public:
  parser() {}

  virtual ~parser() {}

  virtual uint32_t parse(const std::string &enclave_path,
                         const ypc::bytes &param);

  virtual uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                                   uint8_t **data, uint32_t *len) {
    return 0;
  }

  inline std::shared_ptr<keymgr_sgx_module> keymgr() const { return m_keymgr; }

  const ypc::bytes &result() const { return m_result; }

protected:
  ypc::bytes m_result;
  std::shared_ptr<ypc::parser_sgx_module> m_parser;
  std::shared_ptr<keymgr_sgx_module> m_keymgr;
};

uint32_t parser::parse(const std::string &enclave_path,
                       const ypc::bytes &param) {
  auto parser_enclave_path = std::string(PROJECT_SOURCE_DIR) + enclave_path;
  auto keymgr_enclave_path =
      std::string(PROJECT_SOURCE_DIR) + std::string("/lib/keymgr.signed.so");
  m_parser =
      std::make_shared<ypc::parser_sgx_module>(parser_enclave_path.c_str());
  m_keymgr = std::make_shared<keymgr_sgx_module>(keymgr_enclave_path.c_str());

  LOG(INFO) << "initializing parser/keymgr module done";

  auto ret = m_parser->init();
  if (ret != stx_status::success) {
    LOG(ERROR) << "init, got error: " << ypc::status_string(ret);
    return ret;
  }
  ret = m_parser->init_parser();
  if (ret != stx_status::success) {
    LOG(ERROR) << "init_parser, got error: " << ypc::status_string(ret);
    return ret;
  }

  ntt::param_t param_var;
  param_var.set<ntt::param_data>(param);
  typename ypc::cast_obj_to_package<ntt::param_t>::type param_pkg = param_var;
  auto param_bytes = ypc::make_bytes<ypc::bytes>::for_package(param_pkg);
  ret = m_parser->parse_data_item(param_bytes.data(), param_bytes.size());
  if (ret != 0u) {
    LOG(ERROR) << "parse_data_item, got error: " << ypc::status_string(ret);
    return ret;
  }

  ypc::bytes res;
  ret = m_parser->get_analyze_result(res);
  if (ret != 0u) {
    LOG(ERROR) << "get_analyze_result got error " << ypc::status_string(ret);
    return ret;
  }
  m_result = res;

  ret = m_parser->finalize();
  if (ret != stx_status::success) {
    LOG(ERROR) << "shutdown, got error: " << ypc::status_string(ret);
  }

  return ypc::success;
}

using stx_status = stbox::stx_status;
std::shared_ptr<parser> g_parser;

extern "C" {
uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1, uint32_t *session_id);
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id);
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size);
uint32_t km_end_session_ocall(uint32_t session_id);

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len);
}

uint32_t km_session_request_ocall(sgx_dh_msg1_t *dh_msg1,
                                  uint32_t *session_id) {
  return g_parser->keymgr()->session_request(dh_msg1, session_id);
}
uint32_t km_exchange_report_ocall(sgx_dh_msg2_t *dh_msg2,
                                  sgx_dh_msg3_t *dh_msg3, uint32_t session_id) {
  return g_parser->keymgr()->exchange_report(dh_msg2, dh_msg3, session_id);
}
uint32_t km_send_request_ocall(uint32_t session_id,
                               secure_message_t *req_message,
                               size_t req_message_size, size_t max_payload_size,
                               secure_message_t *resp_message,
                               size_t resp_message_size) {
  return g_parser->keymgr()->generate_response(req_message, req_message_size,
                                               max_payload_size, resp_message,
                                               resp_message_size, session_id);
}
uint32_t km_end_session_ocall(uint32_t session_id) {
  return g_parser->keymgr()->end_session(session_id);
}

uint32_t next_data_batch(const uint8_t *data_hash, uint32_t hash_size,
                         uint8_t **data, uint32_t *len) {
  return g_parser->next_data_batch(data_hash, hash_size, data, len);
}

TEST(test_sgx_file, inside_blockfile) {
  g_parser.reset(new parser());
  g_parser->parse("/lib/file_operator.signed.so", ypc::bytes());
  auto res = g_parser->result();
  LOG(INFO) << "ret: " << std::string((const char *)res.data(), res.size());
}

typedef ypc::blockfile<0x29384792, 16, 1024> bft;

TEST(test_sgx_file, blockfile_out_gen_read_in) {
  bft f;
  ypc::bytes data_hash = test_m_data(f, "tsf1_out", 150, 102);
  LOG(INFO) << "gen data done " << data_hash;
  // ypc::bytes data_hash = test_m_data(f, "tsf1_out", 15, 10);
  g_parser.reset(new parser());
  g_parser->parse("/lib/file_operator_read.signed.so", data_hash);
}

template <typename FT> ypc::bytes read_data(FT &f, const char *name) {

  f.open_for_read(name);
  if (!f.file().good()) {
    LOG(ERROR) << " cannot open file " << name;
  }

  ypc::bytes kdata_hash;
  ypc::crypto::eth_sgx_crypto::hash_256(ypc::bytes("test"), kdata_hash);

  std::unique_ptr<char[]> buf(new char[FT::BlockSizeLimit]);
  size_t buf_size;
  size_t i = 0;
  while (f.next_item(buf.get(), FT::BlockSizeLimit, buf_size) == FT::succ) {
    ypc::bytes item(buf.get(), buf_size);
    ypc::crypto::eth_sgx_crypto::hash_256(kdata_hash + item, kdata_hash);
    i++;
  }
  f.close();

  return kdata_hash;
}

TEST(test_sgx_file, blockfile_in_gen_read_out) {
  g_parser.reset(new parser());
  g_parser->parse("/lib/file_operator_write.signed.so", ypc::bytes());
  ypc::bytes hash = g_parser->result();
  bft f;
  ypc::bytes actual_hash = read_data(f, "tsf_in_sgx");
  EXPECT_TRUE(hash == actual_hash);
}
