//
// Created by gaowh on 9/15/23.
//

#ifndef YPC_JOB_STEP_HPP
#define YPC_JOB_STEP_HPP

#include "common.hpp"

#include <list>
#include <fstream>

#include <boost/algorithm/string.hpp>

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

        static std::string read_sealed_output(std::string filepath, std::string field)
        {
            std::ifstream ifs(filepath);

            std::string line;
            while (std::getline(ifs, line))
            {
                std::istringstream iss(line);
                std::string iss_data;
                std::getline(iss, iss_data, '=');
                std::string key, value;
                iss >> key >> value;
                boost::trim(key);
                boost::trim(value);
                if (key == field)
                {
                    return value;
                }
            }

            // FIXME: should report error here
            return std::string{""};
        }

        static nlohmann::json forward_message(
                std::string crypto,
                std::string shukey_file,
                std::string dian_pkey,
                std::string enclave_hash,
                std::string forward_result
                )
        {
            nlohmann::json param;
            param["crypto"] = crypto;
            param["forward"] = std::string{""};
            param["use-privatekey-file"] = shukey_file;
            param["tee-pubkey"] = dian_pkey;
            param["output"] = forward_result;

            if (enclave_hash != "")
            {
                param["use-enclave-hash"] = enclave_hash;
            }
            Common::fid_terminus(param);

            std::ifstream f(forward_result);
            nlohmann::json output = nlohmann::json::parse(f);
            return output;
        }

        static void get_first_key(std::string crypto)
        {
            // TODO: fid_keymgr_list
        }
    };
}

#endif //YPC_JOB_STEP_HPP
