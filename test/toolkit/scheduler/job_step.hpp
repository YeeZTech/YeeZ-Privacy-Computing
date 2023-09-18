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
            Common::fid_terminus(param);
            std::ifstream f(shukey_file);
            nlohmann::json data = nlohmann::json::parse(f);
            return data;
        }

        static nlohmann::json seal_data(
                std::string crypto,
                std::string data_url,
                std::string plugin_url,
                std::string sealed_data_url,
                std::string sealed_output,
                std::string data_key_file)
        {
            nlohmann::json param;
            param["crypto"] = crypto;
            param["data-url"] = data_url;
            param["sealed-data-url"] = sealed_data_url;
            param["output"] = sealed_output;
            param["use-publickey-file"] = data_key_file;

            return Common::fid_data_provider(param);
        }

        std::string& trim_string(std::string& in) {
            return in = std::ranges::to<std::string>(in |
                    std::views::drop_while(std::isspace) | std::views::reverse |
                    std::views::drop_while(std::isspace) | std::views::reverse);
        }

        static std::string read_sealed_output(std::string filepath, std::string field)
        {
            std::ifstream ifs(filepath);

            std::string line;
            while (std::getline(ifs, line))
            {
                std::istringstream iss(line);
                std::string iss_data;
                std::get_line(iss, data, '=');
                std::string key, value;
                iss >> key >> value;
                trim_string(key);
                trim_string(value);
                if (key == field)
                {
                    return value;
                }
            }

            // FIXME: should report error here
            return std::string{""};
        }
    };
}

#endif //YPC_JOB_STEP_HPP
