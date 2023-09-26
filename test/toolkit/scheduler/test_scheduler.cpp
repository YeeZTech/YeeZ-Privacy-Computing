#include "test_scheduler.hpp"

using namespace cluster;

nlohmann::json decrypt_result(
        std::string crypto,
        std::string encrypted_result,
        std::string kgt_pkey,
        std::string key_json_list,
        std::string output)
{
    std::string cmd = Common::bin_dir / std::filesystem::path("./result_decrypt") ;
    cmd = cmd +
            " --crypto " + crypto +
            " --encrypted-result " + encrypted_result +
            " --kgt-pkey " + kgt_pkey +
            " --key-json-file " + key_json_list +
            " --output " + output;
    std::string cmd_output = Common::execute_cmd(cmd);

    nlohmann::json ret;
    ret["cmd"] = cmd;
    ret["output"] = cmd_output;
    return ret;
}

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

    std::string data_hash = JobStep::read_sealed_output(sealed_output, "data_id");
    std::string flat_kgt_pkey = JobStep::read_sealed_output(sealed_output, "pkey_kgt");
    summary["data-hash"] = data_hash;
    // print("done seal data with hash: {}, cmd: {}".format(data_hash, r[0]))
    all_outputs.push_back(sealed_data_url);
    all_outputs.push_back(sealed_output);

    std::vector<nlohmann::json> data_forward_json_list;
    for (auto key_file : key_files)
    {
        std::ifstream ifs_kf(key_file);
        nlohmann::json shukey_json = nlohmann::json::parse(ifs_kf);
        std::string forward_result = key_file + ".shukey.foward.json";
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
    spdlog::info("1.2 generate algo key");
    std::string algo_key_file = name + ".algo" + std::to_string(idx) + ".key.json";
    nlohmann::json algo_shukey_json = JobStep::gen_key(crypto, algo_key_file);
    // self.all_outputs.append(algo_key_file)
    key_files.push_back(algo_key_file);
    // 1.3 generate user key
    spdlog::info("1.3 generate user key");
    std::string user_key_file = name + ".user" + std::to_string(idx) + ".key.json";
    nlohmann::json user_shukey_json = JobStep::gen_key(crypto, user_key_file);
    // self.all_outputs.append(user_key_file)
    key_files.push_back(user_key_file);

    // get dian pkey
    spdlog::info("get dian pkey");
    nlohmann::json key = JobStep::get_first_key(crypto);
    std::string pkey = key["public-key"];
    nlohmann::json summary;
    summary["tee-pkey"] = key["public-key"];
    // read parser enclave hash
    spdlog::info("read parser enclave hash");
    std::string enclave_hash = JobStep::read_parser_hash(parser_url);

    // 3. call terminus to generate forward message
    // 3.2 forward algo shu skey
    spdlog::info("3.2 forward algo shu skey");
    std::string algo_forward_result =
            name +
            ".algo" +
            std::to_string(idx) +
            ".shukey.foward.json";
    nlohmann::json algo_forward_json = JobStep::forward_message(
            crypto, algo_key_file, pkey, enclave_hash, algo_forward_result);
    all_outputs.push_back(algo_forward_result);

    // 3.3 forward user shu skey
    spdlog::info("3.3 forward user shu skey");
    std::string user_forward_result = name + ".user" + std::to_string(idx) + ".shukey.foward.json";
    nlohmann::json user_forward_json = JobStep::forward_message(
            crypto, user_key_file, pkey, enclave_hash, user_forward_result);
    all_outputs.push_back(user_forward_result);

    // handle all data
    spdlog::info("handle all data");
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
    spdlog::info("4. call terminus to generate request");
    std::string param_output_url = name + "_param" + std::to_string(idx) + ".json";
    nlohmann::json param_json = JobStep::generate_request(
            crypto, input_param, user_key_file, param_output_url, config);
    summary["analyzer-output"] = param_json["encrypted-input"];
    all_outputs.push_back(param_output_url);

    // 5. call fid_analyzer
    spdlog::info("5. call fid_analyzer");
    std::string parser_input_file = name + "_parser_input.json";
    std::string parser_output_file = name + "_parser_output.json";
    nlohmann::json result_json = JobStep::fid_analyzer_tg(
            user_shukey_json,
            user_forward_json,
            algo_shukey_json,
            algo_forward_json,
            enclave_hash,
            input_data,
            parser_url,
            pkey,
            nlohmann::json(),
            crypto,
            param_json,
            flat_kgt_pkey_list,
            std::vector<uint64_t>(),
            parser_input_file,
            parser_output_file);

    summary["encrypted-result"] = result_json["encrypted_result"];
    summary["result-signature"] = result_json["result_signature"];
    std::string summary_file = name + ".summary.json";
    std::ofstream ofs_sf(summary_file);
    ofs_sf << summary.dump();
    ofs_sf.close();
    all_outputs.push_back(summary_file);
    JobStep::remove_files(all_outputs);

    std::vector<nlohmann::json> key_json_list;
    for (auto key_file : key_files)
    {
        std::ifstream ifs(key_file);
        key_json_list.push_back(nlohmann::json::parse(ifs));
    }
    std::string all_keys_file = name + ".all-keys.json";
    std::ofstream ofs_akf(all_keys_file);
    nlohmann::json kjl;
    kjl["key_pair_list"] = key_json_list;
    // dump
    ofs_akf << kjl.dump();
    ofs_akf.close();

    ret["encrypted_result"] = result_json["encrypted_result"];
    ret["data_kgt_pkey"] = result_json["data_kgt_pkey"];
    ret["all_keys_file"] = all_keys_file;

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

    spdlog::info("build all_tasks");
    std::vector<nlohmann::json> all_tasks;

    nlohmann::json task1;
    task1["name"] = "org_info";
    task1["data"] = nlohmann::json::array({"corp.csv"});
    std::string task1_reader = Common::lib_dir / std::filesystem::path("libt_org_info_reader.so");
    std::string task1_parser = Common::lib_dir / std::filesystem::path("t_org_info_parser.signed.so");
    std::string task1_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"";
    task1["reader"] = nlohmann::json::array({task1_reader});
    task1["parser"] = task1_parser;
    task1["param"] = task1_param;
    all_tasks.push_back(task1);

    nlohmann::json task2;
    task2["name"] = "tax";
    task2["data"] = nlohmann::json::array({"tax.csv"});
    std::string task2_reader = Common::lib_dir / std::filesystem::path("libt_tax_reader.so");
    std::string task2_parser = Common::lib_dir / std::filesystem::path("t_tax_parser.signed.so");
    std::string task2_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"";
    task2["reader"] = nlohmann::json::array({task2_reader});
    task2["parser"] = task2_parser;
    task2["param"] = task2_param;
    all_tasks.push_back(task2);

    nlohmann::json task3;
    task3["name"] = "merge";
    task3["data"] = nlohmann::json::array({"result_org_info.csv", "result_tax.csv"});
    std::string task3_reader1 = Common::lib_dir / std::filesystem::path("libt_org_info_reader.so");
    std::string task3_reader2 = Common::lib_dir / std::filesystem::path("libt_tax_reader.so");
    std::string task3_parser = Common::lib_dir / std::filesystem::path("t_org_tax_parser.signed.so");
    std::string task3_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"";
    task3["reader"] = nlohmann::json::array({task3_reader1, task3_reader2});
    task3["parser"] = task3_parser;
    task3["param"] = task3_param;
    all_tasks.push_back(task3);

//    nlohmann::json all_tasks = nlohmann::json::parse(R"(
//    {
//        "task1": {
//            "name": "org_info",
//            "data": ["corp.csv"],
//            "reader": [os.path.join(common.lib_dir, "libt_org_info_reader.so")],
//            "parser": os.path.join(common.lib_dir, "t_org_info_parser.signed.so"),
//            "param": "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"",
//        },
//        "task2": {
//            "name": "tax",
//            "data": ["tax.csv"],
//            "reader": [os.path.join(common.lib_dir, "libt_tax_reader.so")],
//            "parser": os.path.join(common.lib_dir, "t_tax_parser.signed.so"),
//            "param": "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"",
//        },
//        "task3": {
//            "name": "merge",
//            "data": ["result_org_info.csv", "result_tax.csv"],
//            "reader": [os.path.join(common.lib_dir, "libt_org_info_reader.so"), os.path.join(common.lib_dir, "libt_tax_reader.so")],
//            "parser": os.path.join(common.lib_dir, "t_org_tax_parser.signed.so"),
//            "param": "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"91110114787775909K\\\"}]\"",
//        }
//    }
//    )");

    spdlog::info("build config");
    nlohmann::json config;
    config["request-use-js"] = "true";
    config["remove-files"] = "true";

    spdlog::info("build taskgraph job");
    TaskGraph_Job tj(
            crypto,
            all_tasks,
            std::vector<std::string>(),
            config,
            std::vector<std::string>());
    spdlog::info("run job0");
    tj.run(all_tasks, 0, std::vector<uint64_t>());
    spdlog::info("run job1");
    tj.run(all_tasks, 1, std::vector<uint64_t>());
    spdlog::info("run job2");
    nlohmann::json result = tj.run(all_tasks, 2, std::vector<uint64_t>{0, 1});
    std::string result_file = "taskgraph.result.output";

    return 0; 
}
