#ifndef YEEZ_PRIVACY_COMPUTING_DATAHUB_LIB_H
#define YEEZ_PRIVACY_COMPUTING_DATAHUB_LIB_H

#include "datahub.hpp"

int datahub_cmdline(int argc, char *argv[]); 

int datahub(
    const std::string& crypto = "", 
    const std::string& data_file = "", 
    const std::string& plugin = "", 
    const std::string& use_publickey_hex = "" ,
    const std::string& use_publickey_file = "", 
    const std::string& sealed_data_file = "", 
    const std::string& output = ""); 

#endif //YEEZ_PRIVACY_COMPUTING_DATAHUB_LIB_H