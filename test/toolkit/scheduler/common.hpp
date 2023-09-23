//
// Created by gaowh on 9/15/23.
//

#ifndef YPC_COMMON_HPP
#define YPC_COMMON_HPP

#include "project.hpp"

#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <fstream>

#include "nlohmann/json.hpp"
#include <boost/algorithm/string.hpp>
#include "spdlog/spdlog.h"

namespace cluster {
    class Common {

    public:
        Common() {
            sdk_dir = std::filesystem::current_path();
            bin_dir = sdk_dir / std::filesystem::path("./bin");
            lib_dir = sdk_dir / std::filesystem::path("./lib");
//            kmgr_enclave.stdeth = lib_dir / std::filesystem::path("keymgr.signed.so");
//            kmgr_enclave.gmssl = lib_dir / std::filesystem::path("keymgr_gmssl.signed.so");
            kmgr_enclave["stdeth"] = lib_dir / std::filesystem::path("keymgr.signed.so");
            kmgr_enclave["gmssl"] = lib_dir / std::filesystem::path("keymgr_gmssl.signed.so");
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

        static nlohmann::json fid_keymgr_list(std::string crypto = std::string{"stdeth"})
        {
            std::string cmd = bin_dir / std::filesystem::path("./keymgr_tool");
            cmd = cmd + " --crypto " + crypto;
            std::string output = execute_cmd(cmd + " --list");

            std::vector<std::string> svec_output;
            std::istringstream iss_output(output);
            std::string s_output;

            std::string tkeyid = std::string{""};
            nlohmann::json keys;

            while (getline(iss_output, s_output, '\n')) {
                svec_output.push_back(s_output);
            }
            for (auto iter_output : svec_output)
            {
                boost::trim(iter_output);
                if (iter_output.rfind(">> key ", 0) == 0)
                {
                    std::vector<std::string> svec_iter_output;
                    std::istringstream iss_iter_output(iter_output);
                    std::string s_iter_output;
                    while (getline(iss_iter_output, s_iter_output, ':'))
                    {
                        svec_iter_output.push_back(s_iter_output);
                        boost::trim(svec_iter_output[1]);
                        tkeyid = svec_iter_output[1];
                    }
                }
                if (iter_output.rfind("public key:", 0) == 0)
                {
                    std::vector<std::string> svec_iter_output;
                    std::istringstream iss_iter_output(iter_output);
                    std::string s_iter_output;
                    while (getline(iss_iter_output, s_iter_output, ':'))
                    {
                        svec_iter_output.push_back(s_iter_output);
                        boost::trim(svec_iter_output[1]);
                        std::string pkey = svec_iter_output[1];
                        if (pkey.rfind(tkeyid, 0) == 0)
                        {
                            keys[tkeyid] = pkey;
                        }
                    }
                }
            }

            return keys;
        }

        static nlohmann::json fid_keymgr_create(std::string user_id, std::string crypto = "")
        {
            nlohmann::json ret;

            std::string cmd = bin_dir / std::filesystem::path("./keymgr_tool");
            cmd = cmd + " --crypto " + crypto;

            nlohmann::json param;
            param["create"] = "";
            param["user-id"] = user_id;

            for (nlohmann::json::iterator iter = param.begin(); iter != param.end(); ++iter)
            {
                cmd = cmd + " --" + iter.key() + " " + to_string(iter.value());
            }

            std::string output = execute_cmd(cmd);

            ret["cmd"] = cmd;
            ret["output"] = output;

            return ret;
        }

        static std::string get_keymgr_private_key(std::string keyid, std::string crypto_type = "stdeth")
        {
            std::string cmd = bin_dir / std::filesystem::path("./keymgr_tool");
            cmd = cmd + " --crypto " + crypto_type;
            std::string output = execute_cmd(cmd + " --list");

            std::vector<std::string> ls;
            boost::split(ls, output, boost::is_any_of("\n"));

            std::vector<std::string> ks;
            boost::split(ks, ls[0], boost::is_any_of(" "));
            std::string dir_ = ks[ks.size() - 1];
            std::string fp = std::filesystem::path(dir_) / std::filesystem::path(keyid);

            std::ifstream ifs(fp);
            nlohmann::json info = nlohmann::json::parse(ifs);

            return info["private_key"].template get<std::string>();
        }

        static nlohmann::json fid_dump(nlohmann::json param)
        {
            nlohmann::json ret;

            std::string cmd = bin_dir / std::filesystem::path("./ydump");
            for (nlohmann::json::iterator iter = param.begin(); iter != param.end(); ++iter)
            {
                cmd = cmd + " --" + iter.key() + " " + to_string(iter.value());
            }
            std::string output = execute_cmd(cmd);

            ret["cmd"] = cmd;
            ret["output"] = output;

            return ret;
        }

        static nlohmann::json fid_analyzer(nlohmann::json param)
        {
            nlohmann::json ret;

            std::string cmd = bin_dir / std::filesystem::path("./fid_analyzer");
            cmd = "GLOG_logtostderr=1 " + cmd;
            for (nlohmann::json::iterator iter = param.begin(); iter != param.end(); ++iter)
            {
                cmd = cmd + " --" + iter.key() + " " + to_string(iter.value());
            }
            std::string output = execute_cmd(cmd);

            ret["cmd"] = cmd;
            ret["output"] = output;

            return ret;
        }

    public:
        inline static std::string sdk_dir;
        inline static std::string bin_dir;
        inline static std::string lib_dir;
//        inline static struct {
//            std::string stdeth;
//            std::string gmssl;
//        } kmgr_enclave;
        inline static nlohmann::json kmgr_enclave;
    };
}

#endif //YPC_COMMON_HPP
