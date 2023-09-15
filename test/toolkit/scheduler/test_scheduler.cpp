#include "test_scheduler.hpp"

using namespace cluster;

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

    return 0; 
}
