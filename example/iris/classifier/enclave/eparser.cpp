#include "corecommon/crypto/stdeth.h"
#include "enclave_iris_classifier.h"
#include "ypc_t/analyzer/algo_wrapper.h"
#include "ypc_t/analyzer/macro.h"

ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::noinput_data_stream,
                  enclave_iris_classifier, ypc::local_result, means_t>
    pw;

YPC_PARSER_IMPL(pw);
