#include "corecommon/crypto/stdeth.h"
#include "first_match_parser.h"
#include "ypc_t/analyzer/algo_wrapper.h"
#include "ypc_t/analyzer/macro.h"

ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::sealed_data_stream,
                  first_match_parser,
                  ypc::onchain_result<ypc::crypto::eth_sgx_crypto>>
    pw;

YPC_PARSER_IMPL(pw);
