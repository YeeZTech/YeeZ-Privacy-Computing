#include "enclave_iris_parser.h"
#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/core_t/analyzer/interface/allowance_interface.h"

using Crypto = ypc::crypto::eth_sgx_crypto;
// using Crypto = ypc::crypto::gmssl_sgx_crypto;

ypc::algo_wrapper<Crypto, ypc::sealed_data_stream, enclave_iris_means_parser,
                  ypc::onchain_result<Crypto>, void, ypc::check_data_allowance>
    pw;

YPC_PARSER_IMPL(pw);
