#!/usr/bin/python3
import json
import os
import sys
import common
from job_step import job_step


def gen_kgt():
    cmd = os.path.join(common.bin_dir, "./kgt_gen --input kgt-skey.json")
    output = common.execute_cmd(cmd)
    return [cmd, output]


# construct task graph example
# mid_data0 = data0 + algo0 + user0
# mid_data1 = data1 + algo1 + user1
# mid_data2 = mid_data0 + mid_data1 + algo2 + user2


# integrate testcase for one round task graph
class classic_job:
    def __init__(self, crypto, name, data_url, parser_url, plugin_url, input_param, config={}):
        self.crypto = crypto
        self.name = name
        self.data_url = data_url
        self.parser_url = parser_url
        self.plugin_url = plugin_url
        self.input = input_param
        self.all_outputs = list()
        self.config = config

    def __generate_keys(self, l, role, n):
        for i in range(n):
            key_file = self.name + ".{}_{}.key.json".format(role, i)
            if not os.path.exists(key_file):
                shukey_json = job_step.gen_key(self.crypto, key_file)
            with open(key_file, 'r') as f:
                shukey_json = json.load(f)
            l.append(shukey_json)
            self.all_outputs.append(key_file)

    def __construct_kgt(self, ldata, lalgo, luser):
        kgt_data_1 = {"value": ldata[0], "children": []}
        kgt_algo_1 = {"value": lalgo[0], "children": []}
        kgt_user_1 = {"value": luser[0], "children": []}
        kgt_middata_1 = {"value": "", "children": [
            kgt_data_1, kgt_algo_1, kgt_user_1]}

        kgt_data_2 = {"value": ldata[1], "children": []}
        kgt_algo_2 = {"value": lalgo[1], "children": []}
        kgt_user_2 = {"value": luser[1], "children": []}
        kgt_middata_2 = {"value": "", "children": [
            kgt_data_2, kgt_algo_2, kgt_user_2]}

        kgt_algo_3 = {"value": lalgo[2], "children": []}
        kgt_user_3 = {"value": luser[2], "children": []}
        kgt_middata_3 = {"value": "", "children": [
            kgt_middata_1, kgt_middata_2, kgt_algo_3, kgt_user_3]}
        return kgt_middata_3

    def construct_kgt(self, key_type):
        data_shukey_json_list = list()
        self.__generate_keys(data_shukey_json_list, "data", 2)
        algo_shukey_json_list = list()
        self.__generate_keys(algo_shukey_json_list, "algo", 3)
        user_shukey_json_list = list()
        self.__generate_keys(user_shukey_json_list, "user", 3)

        kgt = self.__construct_kgt([item[key_type]for item in data_shukey_json_list], [item[key_type]
                                   for item in algo_shukey_json_list], [item[key_type]for item in user_shukey_json_list])
        with open("kgt-skey.json".format(key_type), "w") as f:
            json.dump(kgt, f)
        return data_shukey_json_list, algo_shukey_json_list, user_shukey_json_list

    def run(self):
        # generate kgt
        d_list, a_list, u_list = self.construct_kgt('private-key')
        kgt_shukey_list = list()
        kgt_shukey_list.extend(d_list)
        kgt_shukey_list.extend(a_list)
        kgt_shukey_list.extend(u_list)
        gen_kgt()
        with open('kgt-pkey.json', 'r') as f:
            flat_kgt_pkey = json.load(f)['flat-kgt']
        # 1. generate keys
        # 1.1 generate data key
        # TODO
        data_key_file = "kgt-sum.json"
        with open(data_key_file, 'r') as f:
            data_shukey_json = json.load(f)
        # data_shukey_json = job_step.gen_key(self.crypto, data_key_file)
        self.all_outputs.append(data_key_file)
        # 1.2 generate algo key
        algo_key_file = self.name + ".algo.key.json"
        algo_shukey_json = job_step.gen_key(self.crypto, algo_key_file)
        self.all_outputs.append(algo_key_file)
        # 1.3 generate user key
        key_file = self.name + ".key.json"
        shukey_json = job_step.gen_key(self.crypto, key_file)
        self.all_outputs.append(key_file)

        # 2. call data provider to seal data
        sealed_data_url = self.name + ".sealed"
        sealed_output = self.name + ".sealed.output"
        summary = {}
        summary['data-url'] = self.data_url
        summary['plugin-path'] = self.plugin_url
        summary['sealed-data-url'] = sealed_data_url
        summary['sealed-output'] = sealed_output

        r = job_step.seal_data(self.crypto, self.data_url, self.plugin_url,
                               sealed_data_url, sealed_output, data_key_file)
        data_hash = job_step.read_data_hash(sealed_output)
        summary['data-hash'] = data_hash
        print("done seal data with hash: {}, cmd: {}".format(data_hash, r[0]))
        self.all_outputs.append(sealed_data_url)
        self.all_outputs.append(sealed_output)

        # get dian pkey
        key = job_step.get_first_key(self.crypto)
        pkey = key['public-key']
        summary['tee-pkey'] = key['public-key']
        # read parser enclave hash
        enclave_hash = job_step.read_parser_hash(self.parser_url)

        # 3. call terminus to generate forward message
        # TODO to simplify, we directly forward skey-sum
        data_forward_result = self.name + ".data.shukey.foward.json"
        d = job_step.forward_message(
            self.crypto, data_key_file, pkey, enclave_hash, data_forward_result)
        data_forward_json = {
            "shu_pkey": data_shukey_json['public-key'],
            "encrypted_shu_skey": d['encrypted_skey'],
            "shu_forward_signature": d['forward_sig'],
            "enclave_hash": d['enclave_hash'],
        }
        self.all_outputs.append(data_forward_result)

        data_forward_json_list = [data_forward_json]
        for i in range(len(kgt_shukey_list)):
            kgt_shukey_json = kgt_shukey_list[i]
            prefix = kgt_shukey_json['public-key'][:8]
            data_forward_result = self.name + \
                ".{}.shukey.foward.json".format(prefix)
            tmp_key_file = self.name + ".{}.key.json".format(prefix)
            self.all_outputs.append(tmp_key_file)
            with open(tmp_key_file, 'w') as f:
                json.dump(kgt_shukey_json, f)
            d = job_step.forward_message(
                self.crypto, tmp_key_file, pkey, enclave_hash, data_forward_result)
            data_forward_json = {
                "shu_pkey": kgt_shukey_json['public-key'],
                "encrypted_shu_skey": d['encrypted_skey'],
                "shu_forward_signature": d['forward_sig'],
                "enclave_hash": d['enclave_hash'],
            }
            data_forward_json_list.append(data_forward_json)
            self.all_outputs.append(data_forward_result)

        algo_forward_result = self.name + ".algo.shukey.foward.json"
        algo_forward_json = job_step.forward_message(
            self.crypto, algo_key_file, pkey, enclave_hash, algo_forward_result)
        self.all_outputs.append(algo_forward_result)

        param_key_forward_result = self.name + ".request.shukey.foward.json"
        rq_forward_json = job_step.forward_message(
            self.crypto, key_file, pkey, enclave_hash, param_key_forward_result)
        self.all_outputs.append(param_key_forward_result)

        # 4. call terminus to generate request
        param_output_url = self.name + "_param.json"
        param_json = job_step.generate_request(
            self.crypto, self.input, key_file, param_output_url, self.config)
        summary['analyzer-input'] = param_json["encrypted-input"]
        self.all_outputs.append(param_output_url)

        # 5. call fid_analyzer
        data_obj = {
            "input_data_url": sealed_data_url,
            "input_data_hash": data_hash,
            "kgt_shu_info": {
                "kgt_pkey": flat_kgt_pkey,
                "data_shu_infos": data_forward_json_list,
            },
            "tag": "0"
        }
        input_data = [data_obj]

        parser_input_file = self.name + "parser_input.json"
        parser_output_file = self.name + "parser_output.json"
        result_json = job_step.fid_analyzer_tg(shukey_json, rq_forward_json, algo_shukey_json, algo_forward_json, enclave_hash, input_data, self.parser_url, pkey, dict(
        ), self.crypto, param_json, flat_kgt_pkey, list(), parser_input_file, parser_output_file)

        summary['encrypted-result'] = result_json["encrypted_result"]
        summary["result-signature"] = result_json["result_signature"]
        with open(self.name + ".summary.json", "w") as of:
            json.dump(summary, of)
        self.all_outputs.append(parser_input_file)
        self.all_outputs.append(parser_output_file)
        job_step.remove_files(self.all_outputs)


def intermediate_seal_data(encrypted_data, sealed_data_url):
    cmd = os.path.join(
        common.bin_dir, "./intermediate_data_provider --encrypted-data-hex {} --sealed-data-url {}".format(encrypted_data, sealed_data_url))
    output = common.execute_cmd(cmd)
    return [cmd, output]


def decrypt_result(crypto, encrypted_result, kgt_pkey, key_json_list, output):
    cmd = os.path.join(
        common.bin_dir, "./result_decrypt --crypto {} --encrypted-result {} --kgt-pkey {} --key-json-file {} --output {}".format(crypto, encrypted_result, kgt_pkey, key_json_list, output))
    output = common.execute_cmd(cmd)
    return [cmd, output]


# integrate testcase for multi round task graph
class taskgraph_job:
    def __init__(self, crypto, all_tasks, config={}):
        self.crypto = crypto
        self.all_tasks = all_tasks
        self.key_files = list()
        self.all_outputs = list()
        self.config = config

    def handle_input_data(self, summary, data_url, plugin_url, dian_pkey, enclave_hash, idx, tasks, prev_tasks_idx):
        # 1.1 generate data key
        data_key_file = data_url + ".data{}.key.json".format(idx)
        data_shukey_json = job_step.gen_key(self.crypto, data_key_file)
        # self.all_outputs.append(data_key_file)
        self.key_files.append(data_key_file)

        # 2. call data provider to seal data
        sealed_data_url = data_url + ".sealed"
        sealed_output = data_url + ".sealed.output"
        summary['data-url'] = data_url
        summary['plugin-path'] = plugin_url
        summary['sealed-data-url'] = sealed_data_url
        summary['sealed-output'] = sealed_output

        r, data_hash, flat_kgt_pkey = str(), str(), str()
        if not prev_tasks_idx:
            r = job_step.seal_data(
                self.crypto, data_url, plugin_url, sealed_data_url, sealed_output, data_key_file)
        else:
            taski = tasks[idx]
            name = taski['name']
            parser_output_file = name + "_parser_output.json"
            with open(parser_output_file) as fp:
                output_json = json.load(fp)
            r = intermediate_seal_data(
                output_json['encrypted_result'], sealed_data_url)
            with open(sealed_output, 'w') as fp:
                fp.write('data_id = {}\n'.format(
                    output_json['intermediate_data_hash']))
                fp.write('pkey_kgt = {}\n'.format(
                    output_json['data_kgt_pkey']))
        data_hash = job_step.read_sealed_output(sealed_output, 'data_id')
        flat_kgt_pkey = job_step.read_sealed_output(sealed_output, 'pkey_kgt')
        summary['data-hash'] = data_hash
        print("done seal data with hash: {}, cmd: {}".format(data_hash, r[0]))
        self.all_outputs.append(sealed_data_url)
        self.all_outputs.append(sealed_output)

        data_forward_json_list = []
        for key_file in self.key_files:
            with open(key_file) as fp:
                shukey_json = json.load(fp)
            forward_result = key_file + ".shukey.foward.json"
            d = job_step.forward_message(
                self.crypto, key_file, dian_pkey, enclave_hash, forward_result)
            forward_json = {
                "shu_pkey": shukey_json['public-key'],
                "encrypted_shu_skey": d['encrypted_skey'],
                "shu_forward_signature": d['forward_sig'],
                "enclave_hash": d['enclave_hash'],
            }
            self.all_outputs.append(forward_result)
            data_forward_json_list.append(forward_json)

        data_obj = {
            "input_data_url": sealed_data_url,
            "input_data_hash": data_hash,
            "kgt_shu_info": {
                "kgt_pkey": flat_kgt_pkey,
                "data_shu_infos": data_forward_json_list,
            },
            "tag": "0"
        }
        return data_obj, flat_kgt_pkey

    def run(self, tasks, idx, prev_tasks_idx):
        task = tasks[idx]
        name = task['name']
        data_urls = task['data']
        plugin_urls = task['reader']
        parser_url = task['parser']
        input_param = task['param']

        key_file_names = name + '.key_file_names.json'
        for ti in prev_tasks_idx:
            t = tasks[ti]
            with open(t['name'] + '.key_file_names.json') as f:
                self.key_files.extend(json.load(f))

        # 1. generate keys
        # 1.2 generate algo key
        algo_key_file = name + ".algo{}.key.json".format(idx)
        algo_shukey_json = job_step.gen_key(self.crypto, algo_key_file)
        # self.all_outputs.append(algo_key_file)
        self.key_files.append(algo_key_file)
        # 1.3 generate user key
        user_key_file = name + ".user{}.key.json".format(idx)
        user_shukey_json = job_step.gen_key(self.crypto, user_key_file)
        # self.all_outputs.append(user_key_file)
        self.key_files.append(user_key_file)

        # get dian pkey
        key = job_step.get_first_key(self.crypto)
        pkey = key['public-key']
        summary = {}
        summary['tee-pkey'] = key['public-key']
        # read parser enclave hash
        enclave_hash = job_step.read_parser_hash(parser_url)

        # 3. call terminus to generate forward message
        # 3.2 forward algo shu skey
        algo_forward_result = name + ".algo{}.shukey.foward.json".format(idx)
        algo_forward_json = job_step.forward_message(
            self.crypto, algo_key_file, pkey, enclave_hash, algo_forward_result)
        self.all_outputs.append(algo_forward_result)

        # 3.3 forward user shu skey
        user_forward_result = name + ".user{}.shukey.foward.json".format(idx)
        user_forward_json = job_step.forward_message(
            self.crypto, user_key_file, pkey, enclave_hash, user_forward_result)
        self.all_outputs.append(user_forward_result)

        # handle all data
        if prev_tasks_idx:
            assert len(prev_tasks_idx) == len(data_urls)
        input_data = []
        flat_kgt_pkey_list = []
        for idx, _ in enumerate(data_urls):
            data_obj, flat_kgt_pkey = self.handle_input_data(
                summary, data_urls[idx], plugin_urls[idx], pkey, enclave_hash, idx, tasks, prev_tasks_idx)
            input_data.append(data_obj)
            flat_kgt_pkey_list.append(flat_kgt_pkey)

        # 4. call terminus to generate request
        param_output_url = name + "_param{}.json".format(idx)
        param_json = job_step.generate_request(
            self.crypto, input_param, user_key_file, param_output_url, self.config)
        summary['analyzer-input'] = param_json["encrypted-input"]
        self.all_outputs.append(param_output_url)

        # 5. call fid_analyzer
        parser_input_file = name + "_parser_input.json"
        parser_output_file = name + "_parser_output.json"
        result_json = job_step.fid_analyzer_tg(user_shukey_json, user_forward_json, algo_shukey_json, algo_forward_json, enclave_hash, input_data, parser_url, pkey, dict(
        ), self.crypto, param_json, flat_kgt_pkey_list, list(), parser_input_file, parser_output_file)

        summary['encrypted-result'] = result_json["encrypted_result"]
        summary["result-signature"] = result_json["result_signature"]
        summary_file = name + ".summary.json"
        with open(summary_file, "w") as of:
            json.dump(summary, of)
        # self.all_outputs.append(parser_input_file)
        # self.all_outputs.append(parser_output_file)
        self.all_outputs.append(summary_file)
        job_step.remove_files(self.all_outputs)

        with open(key_file_names, 'w') as f:
            json.dump(self.key_files, f)
        key_json_list = list()
        for key_file in self.key_files:
            with open(key_file) as fp:
                key_json_list.append(json.load(fp))
        all_keys_file = name + ".all-keys.json"
        with open(all_keys_file, 'w') as fp:
            json.dump({'key_pair_list': key_json_list}, fp)
        return result_json['encrypted_result'], result_json['data_kgt_pkey'], all_keys_file


def main():
    crypto = "stdeth"
    all_tasks = [
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
        },
    ]
    tj = taskgraph_job(crypto, all_tasks, {
        'request-use-js': True,
        'remove-files': True if len(sys.argv) < 2 else False,
    })
    # tj.run(all_tasks, 0, [])
    tj.run(all_tasks, 1, [])
    # enc_res, kgt_pkey, all_keys_file = tj.run(all_tasks, 2, [0, 1])
    # result_file = 'taskgraph.result.output'
    # decrypt_result(crypto, enc_res, kgt_pkey, all_keys_file, result_file)
    # with open(result_file) as fp:
    #     print('\n\ndecrypted result is:')
    #     print(fp.read())


if __name__ == '__main__':
    main()
