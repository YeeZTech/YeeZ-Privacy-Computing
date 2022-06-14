#include "../enclave/enclave_iris_parser.h"
#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/stdeth.h"

ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::sealed_data_stream,
                  enclave_iris_parser,
                  ypc::offchain_result<ypc::crypto::eth_sgx_crypto>>
    pw;

YPC_PARSER_IMPL(pw);

