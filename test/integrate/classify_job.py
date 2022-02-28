#!/usr/bin/python3
import common
import os
import sys
import json

def get_first_key():
    keys = common.fid_keymgr_list()

    if len(keys) == 0:
        common.fid_keymgr_create("test")

    keys = common.fid_keymgr_list()
    pkey = ''
    private_key = ''
    for k, v in keys.items():
        pkey = v
        private_key = common.get_keymgr_private_key(k)
        break;
    return {'public-key':pkey, "private-key":private_key}

class classic_job:

    def __init__(self, name, parser_url, input_param):
        self.name = name
        self.parser_url = parser_url
        self.input = input_param

    def run(self):
        '''1. call terminus to generate key,
        2. call terminus to generate forward message
        3. call iris_model to generate model
        4. call terminus to encrypt model

        5. call iris_data to generate input

        6. call terminus to generate allowance

        7. call fid_analyzer

        '''
        summary={}

        #1. generate key
        key_file = self.name + ".key.json"
        param = {"gen-key": "",
                "no-password":"",
                "output":key_file}
        common.fid_terminus(**param)
        shukey_json= {}
        with open(key_file, 'r') as of:
            shukey_json = json.load(of)


        #use first pkey
        key = get_first_key()
        pkey = key['public-key']
        summary['tee-pkey'] = key['public-key']

        #2. call terminusto generate forward message
        forward_result = self.name + ".shukey.foward.json"
        param = {"forward":"",
                "use-privatekey-file":key_file,
                "tee-pubkey":pkey,
                'use-enclave-hash':self.read_parser_hash(),
                "output":forward_result}
        common.fid_terminus(**param);
        data_forward_json = {}
        with open(forward_result, 'r') as of:
            data_forward_json = json.load(of)

        #3. call iris_model
        param = {}
        r = common.iris_model(**param)
        model = r[1]

        #4. encrypt
        encrypt_file = self.name + ".classifier.encrypt.json"
        param = {"encrypt":"",
                "use-publickey-file":key_file,
                "output":encrypt_file,
                "use-param":model
                }
        r = common.fid_terminus(**param);
        encrypted_model= ""
        with open(encrypt_file, 'r') as of:
            encrypted_model = of.readlines()[0]
        print('encrypt result: ', encrypted_model)

        #5. gen input
        param = self.input

        r = common.iris_data(**param)
        iris_input = r[1]
        param = {"sha3":"",
                "use-param":iris_input}
        r = common.fid_terminus(**param)
        param_hash = r[1]

        #6.0 call terminus to generate model hash
        param = {"sha3":"",
                "use-param":model}
        r = common.fid_terminus(**param)
        model_hash = r[1]

        #6. call terminus to generate allowance
        allowance_file = self.name + ".classify.allowance.json"
        param ={"allowance":"",
                "use-privatekey-file":key_file,
                "output":allowance_file,
                "use-param":param_hash.strip(),
                "use-enclave-hash":self.read_parser_hash(),
                "tee-pubkey":pkey,
                "dhash":model_hash
                }
        common.fid_terminus(**param)
        allowance_json = {}
        with open(allowance_file, 'r') as of:
            allowance_json = json.load(of)

        #7. call fid_analyzer

        result_url = self.name + ".classify.result"
        parser_input = {"shu_info":{
            "shu_pkey":shukey_json["public-key"],
            "encrypted_shu_skey":data_forward_json["encrypted_skey"],
            "shu_forward_signature":data_forward_json["forward_sig"],
            "enclave_hash":data_forward_json["enclave_hash"]
            },
            "input_data":"",
            "parser_path":self.parser_url,
            "keymgr_path":common.kmgr_enclave,
            "parser_enclave_hash":self.read_parser_hash(),
            "dian_pkey":pkey,
            "model":{
                "model_data":encrypted_model,
                "public-key":shukey_json["public-key"]
                },
            "param":{
                "param_data":iris_input,
                "public-key":"",
                "allowances":[{
                    "encrypted_sig":allowance_json["signature"],
                    "public-key":allowance_json["pkey"],
                    "data_hash":model_hash
                    }]
                }}

        parser_input_file = self.name + ".classifier.parser_input.json"
        with open(parser_input_file, "w") as of:
            json.dump(parser_input, of)

        parser_output_file = self.name + ".classifier.parser_output.json"

        param = {"input":parser_input_file,
                "output":parser_output_file
                }
        r = common.fid_analyzer(**param);
        print("done fid_analyzer with cmd: {}".format(r[0]))


        with open(parser_output_file) as f:
            self.result = f.readlines();

        print('result: ', self.result)


    @staticmethod
    def read_data_hash(fp):
        with open(fp) as f:
            lines = f.readlines()
            for l in lines:
                if l.startswith("data_id"):
                    ks = l.split("=")
                    return ks[1].strip()

        pass

    def read_parser_hash(self):
        param = {"enclave":self.parser_url,
                "output": "info.json"}
        r = common.fid_dump(**param)
        with open("info.json") as f:
            data = json.load(f)
            return data["enclave-hash"]

if __name__ == "__main__":
    name = "iris_classify"
    # data = os.path.join(common.bin_dir, "iris.data")
    parser = os.path.join(common.lib_dir, "iris_classifier.signed.so")
    # plugin = os.path.join("~/output/iris/output/libiris_csv_reader.so")
    input_param = {"sepal-len":"5.1",
                "sepal-wid":"3.5",
                "petal-len":"1.4",
                "petal-wid":"0.2"}
    cj = classic_job(name, parser, input_param)
    cj.run()
