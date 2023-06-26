#!/usr/bin/python3
import json
import os
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
            shukey_json = job_step.gen_key(self.crypto, key_file)
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
                ".{}.data.shukey.foward.json".format(prefix)
            tmp_key_file = self.name + ".{}.data.key.json".format(prefix)
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
                "kgt_pkey_sum": data_shukey_json["public-key"],
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
