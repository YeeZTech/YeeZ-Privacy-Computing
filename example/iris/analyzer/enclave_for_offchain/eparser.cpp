#include "../enclave/enclave_iris_parser.h"
#include "corecommon/crypto/stdeth.h"
#include "ypc_t/analyzer/algo_wrapper.h"
#include "ypc_t/analyzer/macro.h"

ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::sealed_data_stream,
                  enclave_iris_parser,
                  ypc::offchain_result<ypc::crypto::eth_sgx_crypto>>
    pw;

YPC_PARSER_IMPL(pw);

