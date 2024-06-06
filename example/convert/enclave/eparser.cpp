#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/crypto/gmssl.h"

#include "convert_parser.h"

ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::convert_sealed_data_stream,
                  convert_parser,
                  ypc::onchain_result<ypc::crypto::eth_sgx_crypto>>
    pw;

YPC_PARSER_IMPL(pw);