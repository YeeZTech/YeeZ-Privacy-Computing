//
// Created by gaowh on 9/13/23.
//

#ifndef YEEZ_PRIVACY_COMPUTING_YDUMP_LIB_H
#define YEEZ_PRIVACY_COMPUTING_YDUMP_LIB_H

#include "ydump.hpp"

int ydump_cmdline(int argc, char *argv[]); 

int ydump(
    const std::string& etype = "parser", 
    const std::string& enclave_path = "", 
    const std::string& output_path = ""); 

#endif //YEEZ_PRIVACY_COMPUTING_YDUMP_LIB_H
