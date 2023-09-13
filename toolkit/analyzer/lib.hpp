#ifndef YEEZ_PRIVACY_COMPUTING_ANALYZER_LIB_H
#define YEEZ_PRIVACY_COMPUTING_ANALYZER_LIB_H

#include "analyzer.hpp"

int analyzer_cmdline(int argc, char *argv[]); 

int analyzer(
    const std::string& input = "", 
    const std::string& output_fp = ""); 

#endif //YEEZ_PRIVACY_COMPUTING_ANALYZER_LIB_H