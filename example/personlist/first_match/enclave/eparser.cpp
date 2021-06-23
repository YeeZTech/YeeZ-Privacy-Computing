#include "eparser_t.h"
#include "first_match_parser.h"
#include "sgx_plugin.h"
#include "sgx_utils.h"
#include "stbox/tsgx/log.h"
#include "ypc_t/analyzer/macro.h"
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>

ypc::parser_wrapper<user_item_t, first_match_parser> pw;

YPC_PARSER_IMPL(pw);
