#!/usr/bin/python3
import common
import json
from job_step import job_step


class classic_job:
    def __init__(self, crypto, name, data_url, parser_url, plugin_url, input_param, config={}):
        self.crypto = crypto
        self.name = name
        self.data_url = data_url
        self.parser_url = parser_url
        self.plugin_url = plugin_url
        self.input = input_param
        self.config = config
        self.all_outputs = list()

    def run(self):
        '''
        1. call terminus to generate key
        2. call data provider to seal data
        3. call terminus to generate forward message
        4. call terminus to generate request
        5. call terminus to generate allowance
        6. call fid_analyzer
        7. call terminus to decrypt
        '''

        # 0. generate key
        data_key_file = self.name + ".data.key.json"
        data_shukey_json = job_step.gen_key(self.crypto, data_key_file)
        self.all_outputs.append(data_key_file)

        # 1. generate key
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

        # use first pkey
        key = job_step.get_first_key(self.crypto)
        pkey = key['public-key']
        summary['tee-pkey'] = key['public-key']

        # 3. call terminus to generate forward message
        forward_result = self.name + ".shukey.foward.json"
        data_forward_json = job_step.forward_message(
            self.crypto, data_key_file, pkey, "", forward_result)
        enclave_hash = job_step.read_parser_hash(self.parser_url)
        self.all_outputs.append(forward_result)

        # 4.0 call terminus to generate forward message
        param_key_forward_result = self.name + ".request.shukey.foward.json"
        rq_forward_json = job_step.forward_message(
            self.crypto, key_file, pkey, enclave_hash, param_key_forward_result)
        self.all_outputs.append(param_key_forward_result)

        # 4.1 call terminus to generate request
        param_output_url = self.name + "_param.json"
        param_json = job_step.generate_request(
            self.crypto, self.input, key_file, param_output_url, self.config)
        summary['analyzer-input'] = param_json["encrypted-input"]
        self.all_outputs.append(param_output_url)

        # 5.0 get param hash
        param_hash_output_url = self.name + "_param.hash"
        param_hash = job_step.hash_256(
            self.crypto, self.input, 'text', param_hash_output_url, self.config)
        self.all_outputs.append(param_hash_output_url)

        # 5.1 call terminus to generate allowance
        allowance_output = self.name + "_allowance.json"
        allowance_json = job_step.generate_allowance(
            self.crypto, param_hash, data_key_file, enclave_hash, pkey, data_hash, allowance_output)
        allowances = [allowance_json]
        self.all_outputs.append(allowance_output)

        input_obj = {
            "input_data_url": sealed_data_url,
            "input_data_hash": data_hash,
            "shu_info": {
                "shu_pkey": data_shukey_json["public-key"],
                "encrypted_shu_skey": data_forward_json["encrypted_skey"],
                "shu_forward_signature": data_forward_json["forward_sig"],
                "enclave_hash": data_forward_json["enclave_hash"]
            },
            "public-key": data_shukey_json["public-key"],
            "tag": "0"
        }
        input_data = [input_obj]

        # 6. call fid_analyzer
        parser_input_file = self.name + "parser_input.json"
        parser_output_file = self.name + "parser_output.json"
        result_json = job_step.fid_analyzer(shukey_json, rq_forward_json, enclave_hash, input_data, self.parser_url, pkey, {
        }, self.crypto, param_json, allowances, self.name, parser_input_file, parser_output_file)

        summary['encrypted-result'] = result_json["encrypted_result"]
        summary["result-signature"] = result_json["result_signature"]
        summary["cost-signature"] = result_json["cost_signature"]
        with open(self.name + ".summary.json", "w") as of:
            json.dump(summary, of)
        self.all_outputs.append(parser_input_file)
        self.all_outputs.append(parser_output_file)

        # 7. call terminus to decrypt
        encrypted_result = summary["encrypted-result"]
        decrypted_result = self.name + ".result"
        self.result = job_step.decrypt_result(
            self.crypto, encrypted_result, key_file, decrypted_result)
        self.all_outputs.append(decrypted_result)
        job_step.remove_files(self.all_outputs)
