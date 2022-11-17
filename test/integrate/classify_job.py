#!/usr/bin/python3
import common
import os
import json
from job_step import job_step


class classic_job:
    def __init__(self, crypto, name, parser_url, input_param, config={}):
        self.crypto = crypto
        self.name = name
        self.parser_url = parser_url
        self.input = input_param
        self.config = config
        self.all_outputs = list()

    def run(self):
        '''
        1. call terminus to generate key
        2. call terminus to generate forward message
        3. call iris_model to generate model
        4. call terminus to encrypt model
        5. call iris_data to generate input
        6. call terminus to generate allowance
        7. call fid_analyzer
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

        # 2. call terminusto generate forward message
        forward_result = self.name + ".shukey.foward.json"
        enclave_hash = job_step.read_parser_hash(self.parser_url)
        data_forward_json = job_step.forward_message(
            self.crypto, key_file, pkey, enclave_hash, forward_result)
        self.all_outputs.append(forward_result)

        # 3. call iris_model
        param = {}
        r = common.iris_model(**param)
        model = r[1]

        # 4. encrypt
        encrypt_file = self.name + ".classifier.encrypt.json"
        encrypted_model = job_step.encrypt_message(
            self.crypto, key_file, model, encrypt_file)
        self.all_outputs.append(encrypt_file)

        # 5. gen input
        param = self.input
        r = common.iris_data(**param)
        iris_input = r[1]
        param_hash_output_url = self.name + "_param.hash"
        param_hash = job_step.hash_256(
            self.crypto, iris_input, 'hex', param_hash_output_url, self.config)
        self.all_outputs.append(param_hash_output_url)

        # 6.0 call terminus to generate model hash
        model_hash_output_url = self.name + "_model.hash"
        model_hash = job_step.hash_256(
            self.crypto, model, 'hex', model_hash_output_url, self.config)
        self.all_outputs.append(model_hash_output_url)

        # 6. call terminus to generate allowance
        allowance_file = self.name + ".classify.allowance.json"
        allowance_json = job_step.generate_allowance(
            self.crypto, param_hash, key_file, enclave_hash, pkey, model_hash, allowance_file)
        self.all_outputs.append(allowance_file)

        # 7. call fid_analyzer
        model = {
            "model_data": encrypted_model,
            "public-key": shukey_json["public-key"]
        }
        allowance_obj = {
            # "encrypted_sig": allowance_json["signature"],
            "public-key": allowance_json["public-key"],
            "data_hash": model_hash,
            "signature": allowance_json["signature"]
        }
        allowances = [allowance_obj]
        parser_input_file = self.name + ".classifier.parser_input.json"
        parser_output_file = self.name + ".classifier.parser_output.json"
        param_json = {
            'encrypted-input': iris_input
        }
        result_json = job_step.fid_analyzer(shukey_json, data_forward_json, enclave_hash, [],
                                            self.parser_url, pkey, model, self.crypto, param_json, allowances, parser_input_file, parser_output_file)
        self.all_outputs.append(parser_input_file)
        self.all_outputs.append(parser_output_file)
        job_step.remove_files(self.all_outputs)
        print(result_json)


if __name__ == "__main__":
    name = "iris_classify"
    crypto = 'stdeth'
    parser = os.path.join(common.lib_dir, "iris_classifier.signed.so")
    input_param = {"sepal-len": "5.1",
                   "sepal-wid": "3.5",
                   "petal-len": "1.4",
                   "petal-wid": "0.2"}
    cj = classic_job(crypto, name, parser, input_param)
    cj.run()
