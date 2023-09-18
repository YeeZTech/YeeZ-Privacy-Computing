#include "test_scheduler.hpp"

using namespace cluster;

TaskGraph_Job::TaskGraph_Job(
        std::string crypto,
        nlohmann::json all_tasks,
        nlohmann::json config) {

}

nlohmann::json TaskGraph_Job::handle_input_data(
        std::string summary,
        std::string data_url,
        std::string plugin_url,
        std::string dian_pkey,
        std::string enclave_hash,
        std::string idx,
        std::string tasks,
        std::string prev_tasks_idx) {
    nlohmann::json ret;

    return ret;
}

nlohmann::json TaskGraph_Job::run(
        std::vector<std::string> tasks,
        uint64_t idx,
        std::vector<uint64_t> prev_tasks_idx) {
    nlohmann::json ret;

    return ret;
}

std::unique_ptr<JobStep> jobStep;
std::unique_ptr<Common> common;

void gen_kgt()
{
    std::string cmd = (common->bin_dir) / std::filesystem::path("./kgt_gen");
    cmd += std::string(" --input kgt-skey.json");
    Common::execute_cmd(cmd);
}

int main(const int argc, const char *argv[]) {
    jobStep = std::make_unique<JobStep>();
    common = std::make_unique<Common>();

    std::string crypto = "stdeth";
    nlohmann::json all_tasks = nlohmann::json::parse(R"(
    {
        {
            'name': 'org_info',
            'data': ['corp.csv'],
            'reader': [os.path.join(common.lib_dir, "libt_org_info_reader.so")],
            'parser': os.path.join(common.lib_dir, "t_org_info_parser.signed.so"),
            'param': "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"",
        },
        {
            'name': 'tax',
            'data': ['tax.csv'],
            'reader': [os.path.join(common.lib_dir, "libt_tax_reader.so")],
            'parser': os.path.join(common.lib_dir, "t_tax_parser.signed.so"),
            'param': "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"",
        },
        {
            'name': 'merge',
            'data': ['result_org_info.csv', 'result_tax.csv'],
            'reader': [os.path.join(common.lib_dir, "libt_org_info_reader.so"), os.path.join(common.lib_dir, "libt_tax_reader.so")],
            'parser': os.path.join(common.lib_dir, "t_org_tax_parser.signed.so"),
            'param': "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"",
        }
    }
    )");

    nlohmann::json config = nlohmann::json::parse(R"(
        "request-use-js": "true",
        "remove-files": "true"
    )");

    TaskGraph_Job tj(crypto, all_tasks, config);
    tj.run(all_tasks, 0, std::vector<uint64_t>());
    tj.run(all_tasks, 1, std::vector<uint64_t>());
    return 0; 
}
