#ifndef YEEZ_PRIVACY_COMPUTING_YTERMINUS_LIB_H
#define YEEZ_PRIVACY_COMPUTING_YTERMINUS_LIB_H

#include "cmd_line.h"

int yterminus_cmdline(int argc, char *argv[]); 

int yterminus(
    const std::string& etype = "parser", 
    const std::string& enclave_path = "", 
    const std::string& output_path = ""); 

#endif //YEEZ_PRIVACY_COMPUTING_YTERMINUS_LIB_H
