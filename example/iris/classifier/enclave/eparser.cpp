#include "enclave_iris_classifier.h"
#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/stdeth.h"

// ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::noinput_data_stream,
// enclave_iris_classifier, ypc::local_result, means_t>
// pw;

ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::noinput_data_stream,
                  enclave_iris_classifier, ypc::local_result, means_t,
                  ypc::ignore_data_allowance, ypc::check_model_allowance>
    pw;

YPC_PARSER_IMPL(pw);
