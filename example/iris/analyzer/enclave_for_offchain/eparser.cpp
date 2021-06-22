#include "../enclave/sgx_plugin.h"
#include "eparser_t.h"
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>

#include "../enclave/enclave_iris_parser.h"
#include "sgx_utils.h"
#include "ypc_t/analyzer/macro.h"

ypc::parser_wrapper_for_offchain<user_item_t, enclave_iris_parser> pw;

YPC_PARSER_IMPL(pw);

