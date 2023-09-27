//
// Created by gaowh on 9/22/23.
//

#ifndef YPC_COMMONJS_HPP
#define YPC_COMMONJS_HPP

#include "nlohmann/json.hpp"

namespace cluster {
    class CommonJs {
    public:
        CommonJs() {
            sdk_dir = std::filesystem::current_path();
            current_dir = sdk_dir / std::filesystem::path("./test/integrate");
            bin_dir = sdk_dir / std::filesystem::path("./bin");
            lib_dir = sdk_dir / std::filesystem::path("./lib");
            kmgr_enclave.stdeth = lib_dir / std::filesystem::path("edatahub.signed.so");
            kmgr_enclave.gmssl = lib_dir / std::filesystem::path("keymgr.signed.so");
        }

        static std::string execute_cmd(std::string cmd)
        {
            spdlog::info("CommonJs::execute_cmd: {}", cmd);

            cmd = "node " + cmd;
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
            spdlog::trace("CommonJs::fid_terminus starts");

            nlohmann::json ret;

            // FIXME: change dir
            std::string cmd = current_dir / std::filesystem::path("./js/simjs.js");
            for (nlohmann::json::iterator iter = kwargs.begin(); iter != kwargs.end(); ++iter)
            {
                cmd = cmd + " --" + iter.key() + " " + to_string(iter.value());
            }

            std::string output = execute_cmd(cmd);

            ret["cmd"] = cmd;
            ret["output"] = output;

            spdlog::trace("CommonJs::fid_terminus ends");

            return  ret;
        }

    public:
        inline static std::string sdk_dir;
        inline static std::string bin_dir;
        inline static std::string lib_dir;
        inline static std::string current_dir;
        inline static struct {
            std::string stdeth;
            std::string gmssl;
        } kmgr_enclave;
    };
}

#endif //YPC_COMMONJS_HPP
