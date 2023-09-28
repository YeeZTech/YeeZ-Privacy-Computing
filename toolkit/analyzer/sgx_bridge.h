#pragma once

#include "parsers/parser.h"
#include "parsers/oram_parser.h"
#include <memory>

extern std::shared_ptr<parser> g_parser;
extern std::shared_ptr<oram_parser> o_parser;
