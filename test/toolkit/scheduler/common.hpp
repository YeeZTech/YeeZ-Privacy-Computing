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
#include <boost/algorithm/string.hpp>

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
//        static std::string execute_cmd(std::string cmd)
//        {
//            std::system(cmd.c_str());
//            return std::string{""};
//        }

        // ack: https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
        static std::string execute_cmd(std::string cmd) {
            auto cmd_cc = cmd.c_str();
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd_cc, "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            return result;
        }

        static nlohmann::json fid_terminus(nlohmann::json kwargs)
        {
            nlohmann::json ret;

            std::string cmd = bin_dir / std::filesystem::path("./yterminus");
            for (nlohmann::json::iterator iter = kwargs.begin(); iter != kwargs.end(); ++iter)
            {
                cmd = cmd + " --" + iter.key() + " " + to_string(iter.value());
            }

            std::string output = execute_cmd(cmd);

            ret["cmd"] = cmd;
            ret["output"] = output;

            return ret;
        }

        static nlohmann::json fid_data_provider(nlohmann::json kwargs)
        {
            nlohmann::json ret;

            std::string cmd = Common::bin_dir / std::filesystem::path("./data_provider");
            for (nlohmann::json::iterator iter = kwargs.begin(); iter != kwargs.end(); ++iter)
            {
                cmd = cmd + " --" + iter.key() + " " + to_string(iter.value());
            }

            std::string output = execute_cmd(cmd);

            ret["cmd"] = cmd;
            ret["output"] = output;

            return ret;
        }

        static std::map<uint64_t, std::string> fid_keymgr_list(std::string crypto = std::string{"stdeth"})
        {
            std::map<uint64_t, std::string> keys;

            // TODO: cmd
            std::string cmd = bin_dir / std::filesystem::path("./keymgr_tool");
            cmd = cmd + " --crypto " + crypto;
            std::string output = execute_cmd(cmd + " --list");

            std::vector<std::string> ls;
            std::istringstream f(output);
            std::string s;

            std::string tkeyid = std::string{""};

            while (getline(f, s, '\n')) {
                boost::trim(s);
                if (s.rfind(">> key ", 0) == 0)
                {
                    // TODO: split
                }
            }

            return keys;
        }

    public:
        inline static std::string sdk_dir;
        inline static std::string bin_dir;
        inline static std::string lib_dir;
        inline static struct {
            std::string stdeth;
            std::string gmssl;
        } kmgr_enclave;
    };
}

#endif //YPC_COMMON_HPP
