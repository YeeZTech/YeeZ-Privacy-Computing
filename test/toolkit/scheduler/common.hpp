//
// Created by gaowh on 9/15/23.
//

#ifndef YPC_COMMON_HPP
#define YPC_COMMON_HPP

#include "project.hpp"

#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <list>

#include "nlohmann/json.hpp"

namespace cluster {
    class Common {

    public:
        Common() {
            sdk_dir = std::filesystem::current_path();
            bin_dir = sdk_dir / std::filesystem::path("./bin");
            lib_dir = sdk_dir / std::filesystem::path("./lib");
            kmgr_enclave.stdeth = lib_dir / std::filesystem::path("keymgr.signed.so");
            kmgr_enclave.gmssl = lib_dir / std::filesystem::path("keymgr_gmssl.signed.so");
        }

    public:
        static void execute_cmd(std::string cmd)
        {
            std::system(cmd.c_str());
        }

        void fid_termius(std::list<std::pair<std::string, std::string>> kwargs)
        {
            std::string cmd = bin_dir / std::filesystem::path("./ydump");
            for (auto iter : kwargs)
            {

            }
        }

    public:
        static std::string sdk_dir;
        static std::string bin_dir;
        static std::string lib_dir;
        static struct {
            std::string stdeth;
            std::string gmssl;
        } kmgr_enclave;
    };
}

#endif //YPC_COMMON_HPP
