#include "eparser_t.h"
#include "sgx_plugin.h"
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>

#include "enclave_iris_parser.h"
#include "sgx_utils.h"
#include "ypc_t/analyzer/macro.h"

ypc::parser_wrapper<user_item_t, enclave_iris_parser> pw;

YPC_PARSER_IMPL(pw, ypc::ecall_parse_item_data);
