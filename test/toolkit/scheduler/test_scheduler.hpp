#ifndef YEEZ_PRIVACY_COMPUTING_TEST_SCHEDULER_H
#define YEEZ_PRIVACY_COMPUTING_TEST_SCHEDULER_H

#include "job_step.hpp"

namespace cluster {
    nlohmann::json intermediate_seal_data(std::string encrypted_data, std::string sealed_data_url)
    {
        nlohmann::json ret;

        std::string cmd = Common::bin_dir / std::filesystem::path("./intermediate_data_provider");
        cmd = cmd + " --encrypted-data-hex ";
        cmd = cmd + encrypted_data;
        cmd = cmd + " --sealed-data-url ";
        cmd = cmd + sealed_data_url;
        std::string output = Common::execute_cmd(cmd);

        ret["cmd"] = cmd;
        ret["output"] = output;
        return ret;
    }

    class TaskGraph_Job {
    public:
        TaskGraph_Job(
                std::string crypto,
                nlohmann::json all_tasks,
                std::vector<std::string> all_output,
                nlohmann::json config,
                std::vector<std::string> key_files);

    public:
        nlohmann::json handle_input_data(
                nlohmann::json summary,
                std::string data_url,
                std::string plugin_url,
                std::string dian_pkey,
                std::string enclave_hash,
                uint64_t idx,
                std::vector<nlohmann::json> tasks,
                std::vector<uint64_t> prev_tasks_idx);

        nlohmann::json run(
                std::vector<std::string> tasks,
                uint64_t idx,
                std::vector<uint64_t> prev_tasks_idx);

    public:
        std::string crypto;
        nlohmann::json all_tasks;
        std::vector<std::string> all_outputs;
        nlohmann::json config;
        std::vector<std::string> key_files;
    };
}

#endif //YEEZ_PRIVACY_COMPUTING_TEST_SCHEDULER_H