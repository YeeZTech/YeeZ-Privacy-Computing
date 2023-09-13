//
// Created by gaowh on 9/13/23.
//

#ifndef YEEZ_PRIVACY_COMPUTING_YDUMP_H
#define YEEZ_PRIVACY_COMPUTING_YDUMP_H

#include "ypc/core/byte.h"
#include "ypc/core/filesystem.h"
#include "ypc/core/sgx/parser_sgx_module.h"
#include "ypc/core/version.h"
#include "ypc/keymgr/common/util.h"
#include "ypc/keymgr/default/keymgr_sgx_module.h"
#include "ypc/stbox/ebyte.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>
#include <unordered_map>

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
    namespace bp = boost::program_options;
    bp::options_description all("YeeZ Privacy Enclave Hash Dumper");

    // clang-format off
    all.add_options()
            ("help", "help message")
            ("version", "show version")
            ("enclave-type", bp::value<std::string>(), "enclave type, can be [keymgr | parser], default is [parser]")
            ("enclave", bp::value<std::string>(), "enclave file path")
            ("output", bp::value<std::string>(), "output result to file with JSON format");

    // clang-format on
    bp::variables_map vm;
    boost::program_options::store(bp::parse_command_line(argc, argv, all), vm);

    if (vm.count("help") != 0u) {
        std::cout << all << std::endl;
        exit(-1);
    }

    if (vm.count("version") != 0u) {
        std::cout << ypc::get_ypc_version() << std::endl;
        exit(-1);
    }

    return vm;
}

void get_enclave_type(uint32_t parser_type, std::unordered_map<std::string, std::string> &result) {
    result["result-type"] = std::to_string(parser_type & 0xf);
    parser_type >>= 4;
    result["data-source-type"] = std::to_string(parser_type & 0xf);
    parser_type >>= 4;
    result["has-model"] = std::to_string(parser_type & 0x1);
}

#endif //YEEZ_PRIVACY_COMPUTING_YDUMP_H
