#!/usr/bin/python3
import common
import commonjs
import json
from job_step import job_step


class multistream_job_offchain:
    def __init__(self, crypto, name, data_urls, parser_url, plugin_url, input_param, config={}):
        self.crypto = crypto
        self.name = name
        self.data_urls = data_urls
        self.parser_url = parser_url
        self.plugin_url = plugin_url
        self.input = input_param
        self.config = config
        self.all_outputs = list()

    def handle_input_data(self, data_url, param_hash):
        '''
        1. generate a key for the data
        2. encrypt the data
        3. gen forward msg
        4. gen allowance
        @return: the input obj and the allowance obj
        '''

        # 1. generate key
        data_key_file = data_url + ".data.key.json"
        data_shukey_json = job_step.gen_key(self.crypto, data_key_file)
        self.all_outputs.append(data_key_file)

        # 2. call data provider to seal data
        sealed_data_url = data_url + ".sealed"
        sealed_output = data_url + ".sealed.output"
        summary = {}
        summary['data-url'] = data_url
        summary['plugin-path'] = self.plugin_url
        summary['sealed-data-url'] = sealed_data_url
        summary['sealed-output'] = sealed_output

        r = job_step.seal_data(self.crypto, data_url, self.plugin_url,
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

        # 3. call terminusto generate forward message
        enclave_hash = job_step.read_parser_hash(self.parser_url)
        forward_result = data_url + ".shukey.foward.json"
        data_forward_json = job_step.forward_message(
            self.crypto, data_key_file, pkey, "", forward_result)
        self.all_outputs.append(forward_result)

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

        # 4. call terminus to generate allowance
        allowance_result = data_url + ".allowance.json"
        allowance_json = job_step.generate_allowance(
            self.crypto, param_hash, data_key_file, enclave_hash, pkey, data_hash, allowance_result)
        self.all_outputs.append(allowance_result)

        return input_obj, allowance_json, data_key_file

    def run(self):
        '''
        1. call terminus to generate key
        2. call terminus to generate request
        3. call terminus to generate allowances
        4. call fid_analyzer
        5. call terminus to decrypt
        '''

        summary = {}
        # 1. generate key
        key_file = self.name + ".key.json"
        shukey_json = job_step.gen_key(self.crypto, key_file)
        self.all_outputs.append(key_file)

        # use first pkey
        key = job_step.get_first_key(self.crypto)
        pkey = key['public-key']
        summary['tee-pkey'] = key['public-key']
        enclave_hash = job_step.read_parser_hash(self.parser_url)

        # 2.0 call terminus to generate forward message
        param_key_forward_result = self.name + ".request.shukey.foward.json"
        rq_forward_json = job_step.forward_message(
            self.crypto, key_file, pkey, enclave_hash, param_key_forward_result)
        self.all_outputs.append(param_key_forward_result)

        # 2.1 call terminus to generate request
        param_output_url = self.name + "_param.json"
        param_json = job_step.generate_request(
            self.crypto, self.input, key_file, param_output_url, self.config)
        summary['analyzer-input'] = param_json["encrypted-input"]
        summary["request_info"] = param_json
        summary["request_info"]["shu_info"] = rq_forward_json
        self.all_outputs.append(param_output_url)

        # 3.0 get param hash
        param_hash_output_url = self.name + "_param_hash.json"
        param_hash = job_step.hash_256(
            self.crypto, self.input, 'text', param_hash_output_url, self.config)
        self.all_outputs.append(param_hash_output_url)

        # 3.1 call terminus to generate allowances
        input_data = []
        allowances = []
        summary["input"] = []
        for data_url in self.data_urls:
            in_data, allowance_data, data_key_file = self.handle_input_data(
                data_url, param_hash)
            input_data.append(in_data)
            allowances.append(allowance_data)
            print("indata: ", in_data)
            ti = in_data.copy()
            rs = job_step.sign(self.crypto, in_data["input_data_hash"], "hex", data_key_file, "temp.json")
            print('rs: ', rs["signature"])
            ti["hash_sig"] =rs["signature"]
            summary["input"].append(ti)

        # 4. call fid_analyzer
        parser_input_file = self.name + "parser_input.json"
        parser_output_file = self.name + "parser_output.json"
        result_json = job_step.fid_analyzer(shukey_json, rq_forward_json, enclave_hash, input_data,
                                            self.parser_url, pkey, {}, self.crypto, param_json, allowances, parser_input_file, parser_output_file)


        summary["enclave_hash"] = enclave_hash
        summary['encrypted-result'] = result_json["encrypted_result"]
        summary['result_encrypt_key'] = result_json['result_encrypt_key']

        summary["result-signature"] = result_json["result_signature"]
        summary["cost-signature"] = result_json["cost_signature"]
        result_hash = job_step.hash_256(
            self.crypto, summary["encrypted-result"], 'hex', "temp.json", '')
        summary["result_hash"] = result_hash

        with open(self.name + ".summary.json", "w") as of:
            json.dump(summary, of)
        self.all_outputs.append(parser_input_file)
        self.all_outputs.append(parser_output_file)

        # 6. call terminus to decrypt
        encrypted_result_key = summary["result_encrypt_key"]
        encrypted_result = summary["encrypted-result"]
        decrypted_result = self.name + ".result"
        print("encrypted result key: ", encrypted_result_key)
        self.result_key = job_step.decrypt_result_key(
            self.crypto, encrypted_result_key, key_file, decrypted_result)

        self.result = job_step.decrypt_result_with_hex(self.crypto, encrypted_result, self.result_key, decrypted_result)

        self.all_outputs.append(decrypted_result)

        # job_step.remove_files(self.all_outputs)
