//
// Created by gaowh on 9/15/23.
//

#ifndef YPC_JOB_STEP_HPP
#define YPC_JOB_STEP_HPP

#include "common.hpp"

#include <list>
#include <fstream>

namespace cluster {
    class JobStep {
    public:
        static void remove_files(std::list<std::string> file_list)
        {
            for (auto iter : file_list)
            {
                std::string cmd = std::string{"rm -rf "} + iter;
                Common::execute_cmd(cmd);
            }

        }

        static nlohmann::json gen_key(std::string crypto, std::string shukey_file)
        {
            nlohmann::json param = nlohmann::json::parse(R"(
                {
                    "crypto": crypto,
                    "gen-key": "",
                    "no-password": "",
                    "output": shukey_file
                }
            )");
            Common::fid_termius(param);
            std::ifstream f(shukey_file);
            nlohmann::json data = nlohmann::json::parse(f);
            return data;
        }

        static nlohmann::json seal_data(
                std::string crypto,
                std::string data_url,
                std::string plugin_url,
                std::string ealed_data_url,
                std::string sealed_output,
                std::string data_key_file)
        {
            nlohmann::json ret;

            return ret;
        }
    };
}

#endif //YPC_JOB_STEP_HPP
