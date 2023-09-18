#ifndef YEEZ_PRIVACY_COMPUTING_TEST_SCHEDULER_H
#define YEEZ_PRIVACY_COMPUTING_TEST_SCHEDULER_H

#include "job_step.hpp"

namespace cluster {
    class TaskGraph_Job {
    public:
        TaskGraph_Job(
                std::string crypto,
                nlohmann::json all_tasks,
                nlohmann::json config);

    public:
        nlohmann::json handle_input_data(
                std::string summary,
                std::string data_url,
                std::string plugin_url,
                std::string dian_pkey,
                std::string enclave_hash,
                std::string idx,
                std::string tasks,
                std::string prev_tasks_idx);

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