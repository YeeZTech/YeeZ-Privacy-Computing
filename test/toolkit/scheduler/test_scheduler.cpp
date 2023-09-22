#include "test_scheduler.hpp"

using namespace cluster;

TaskGraph_Job::TaskGraph_Job(
        std::string crypto,
        nlohmann::json all_tasks,
        std::vector<std::string> all_output,
        nlohmann::json config,
        std::vector<std::string> key_files) :
        crypto(crypto),
        all_tasks(all_tasks),
        all_outputs(all_output),
        config(config),
        key_files(key_files) {

}

nlohmann::json TaskGraph_Job::handle_input_data(
        nlohmann::json summary,
        std::string data_url,
        std::string plugin_url,
        std::string dian_pkey,
        std::string enclave_hash,
        uint64_t idx,
        std::vector<nlohmann::json> tasks,
        std::vector<uint64_t> prev_tasks_idx) {
    nlohmann::json ret;

    // 1.1 generate data key
    std::string data_key_file =
            data_url +
            ".data" +
            std::to_string(idx) +
            ".key.json";
    nlohmann::json data_shukey_json = JobStep::gen_key(crypto, data_key_file);
    key_files.push_back(data_key_file);

    // 2. call data provider to seal data
    std::string sealed_data_url = data_url + ".sealed";
    std::string sealed_output = data_url + ".sealed.output";
    summary["data-url"] = data_url;
    summary["plugin-path"] = plugin_url;
    summary["sealed-data-url"] = sealed_data_url;
    summary["sealed-output"] = sealed_output;

    if (prev_tasks_idx.empty())
    {
        nlohmann::json r = JobStep::seal_data(
                crypto,
                data_url,
                plugin_url,
                sealed_data_url,
                sealed_output,
                data_key_file);
    }
    else
    {
        nlohmann::json task = tasks[idx];
        std::string name = task["name"];
        std::string parser_output_file = name + "_parser_output.json";
        std::ifstream ifs_pof(parser_output_file);
        nlohmann::json output_json = nlohmann::json::parse(ifs_pof);

        nlohmann::json r = intermediate_seal_data(
                output_json["encrypted_result"],
                sealed_data_url);

        std::ofstream ofs_so(sealed_output);
        ofs_so << "data_id = " << output_json["intermediate_data_hash"] << std::endl;
        ofs_so << "pkey_kgt = " << output_json["data_kgt_pkey"] << std::endl;
    }

    // TODO: read sealed output
    std::string data_hash = JobStep::read_sealed_output(sealed_output, "data_id");
    std::string flat_kgt_pkey = JobStep::read_sealed_output(sealed_output, "pkey_kgt");
    summary["data-hash"] = data_hash;
    // TODO: print("done seal data with hash: {}, cmd: {}".format(data_hash, r[0]))
    all_outputs.push_back(sealed_data_url);
    all_outputs.push_back(sealed_output);

    std::vector<nlohmann::json> data_forward_json_list;
    for (auto key_file : key_files)
    {
        std::ifstream ifs_kf(key_file);
        nlohmann::json shukey_json = nlohmann::json::parse(ifs_kf);
        std::string forward_result = key_file + ".shukey.foward.json";
        // TODO: job_step.forward_message
        nlohmann::json d = JobStep::forward_message(
                crypto,
                key_file,
                dian_pkey,
                enclave_hash,
                forward_result);

        nlohmann::json forward_json;
        forward_json["shu_pkey"] = shukey_json["public-key"];
        forward_json["encrypted_shu_skey"] = d["encrypted_skey"];
        forward_json["shu_forward_signature"] = d["forward_sig"];
        forward_json["enclave_hash"] = d["enclave_hash"];

        all_outputs.push_back(forward_result);
        data_forward_json_list.push_back(forward_json);

    }

    nlohmann::json data_obj;
    data_obj["input_data_url"] = sealed_data_url;
    data_obj["input_data_hash"] = data_hash;
    data_obj["input_data_hash"]["kgt_pkey"] = flat_kgt_pkey;
    data_obj["input_data_hash"]["data_shu_infos"] = data_forward_json_list;
    data_obj["tag"] = std::string{"0"};

    ret["data_obj"] = data_obj;
    ret["flat_kgt_pkey"] = flat_kgt_pkey;

    return ret;
}

nlohmann::json TaskGraph_Job::run(
        std::vector<nlohmann::json> tasks,
        uint64_t idx,
        std::vector<uint64_t> prev_tasks_idx) {
    nlohmann::json ret;

    nlohmann::json task = tasks[idx];
    std::string name = task["name"];
    nlohmann::json data_urls = task["data"];
    nlohmann::json plugin_urls = task["reader"];
    std::string parser_url = task["parser"];
    std::string input_param = task["param"];

    // 1. generate keys
    // 1.2 generate algo key
    std::string algo_key_file = name + ".algo" + std::to_string(idx) + ".key.json";
    nlohmann::json algo_shukey_json = JobStep::gen_key(crypto, algo_key_file);
    // self.all_outputs.append(algo_key_file)
    key_files.push_back(algo_key_file);
    // 1.3 generate user key
    std::string user_key_file = name + ".user" + std::to_string(idx) + ".key.json";
    nlohmann::json user_shukey_json = JobStep::gen_key(crypto, user_key_file);
    // self.all_outputs.append(user_key_file)
    key_files.push_back(user_key_file);

    // get dian pkey
    nlohmann::json key = JobStep::get_first_key(crypto);
    std::string pkey = key["public-key"];
    nlohmann::json summary;
    summary["tee-pkey"] = key["public-key"];
    // read parser enclave hash
    std::string enclave_hash = JobStep::read_parser_hash(parser_url);

    // 3. call terminus to generate forward message
    // 3.2 forward algo shu skey
    std::string algo_forward_result =
            name +
            ".algo" +
            std::to_string(idx) +
            ".shukey.foward.json";
    nlohmann::json algo_forward_json = JobStep::forward_message(
            crypto, algo_key_file, pkey, enclave_hash, algo_forward_result);
    all_outputs.push_back(algo_forward_result);

    // 3.3 forward user shu skey
    std::string user_forward_result = name + ".user" + std::to_string(idx) + ".shukey.foward.json";
    nlohmann::json user_forward_json = JobStep::forward_message(
            crypto, user_key_file, pkey, enclave_hash, user_forward_result);
    all_outputs.push_back(user_forward_result);

    // handle all data
    if (!prev_tasks_idx.empty())
    {
        assert(prev_tasks_idx.size() == data_urls.size());
    }
    std::vector<nlohmann::json> input_data;
    std::vector<std::string> flat_kgt_pkey_list;
    for (size_t i = 0; i < data_urls.size(); i++)
    {
        auto iter = data_urls[i];
        nlohmann::json result = handle_input_data(
                summary,
                data_urls[idx],
                plugin_urls[idx],
                pkey, enclave_hash,
                idx,
                tasks,
                prev_tasks_idx);
        input_data.push_back(result["data-obj"]);
        flat_kgt_pkey_list.push_back(result["flat_kgt_pkey"]);
    }

    // 4. call terminus to generate request
    std::string param_output_url = name + "_param" + std::to_string(idx) + ".json";
    nlohmann::json param_json = JobStep::generate_request(
            crypto, input_param, user_key_file, param_output_url, config);
    summary["analyzer-output"] = param_json["encrypted-input"];
    all_outputs.push_back(param_output_url);

    // 5. call fid_analyzer
    std::string parser_input_file = name + "_parser_input.json";
    std::string parser_output_file = name + "_parser_output.json";
    // TODO: fid_analyzer_tg


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

    TaskGraph_Job tj(
            crypto,
            all_tasks,
            std::vector<std::string>(),
            config,
            std::vector<std::string>());
    tj.run(all_tasks, 0, std::vector<uint64_t>());
    tj.run(all_tasks, 1, std::vector<uint64_t>());
    return 0; 
}
