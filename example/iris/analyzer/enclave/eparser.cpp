#include "enclave_iris_parser.h"
#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/gmssl.h"

ypc::algo_wrapper<ypc::crypto::gmssl_sgx_crypto, ypc::sealed_data_stream,
                  enclave_iris_means_parser,
                  ypc::onchain_result<ypc::crypto::gmssl_sgx_crypto>>
    pw;

YPC_PARSER_IMPL(pw);
