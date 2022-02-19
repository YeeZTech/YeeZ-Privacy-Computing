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

    def __init__(self, name, data_url, parser_url, plugin_url, input_param):
        self.name = name
        self.data_url = data_url
        self.parser_url = parser_url
        self.plugin_url = plugin_url
        self.input = input_param

    def run(self):
        '''1. call terminus to generate key,
        2. call data provider to seal data
        3. call terminus to generate forward message

        4. call terminus to generate request
        5. call fid_analyzer
        6. call terminus to decrypt'''

        #0. generate key
        data_key_file = self.name + ".data.key.json"
        param = {"gen-key": "",
                "no-password":"",
                "output":data_key_file}
        common.fid_terminus(**param)
        data_shukey_json= {}
        with open(data_key_file, 'r') as of:
            data_shukey_json = json.load(of)

        #1. generate key
        key_file = self.name + ".key.json"
        param = {"gen-key": "",
                "no-password":"",
                "output":key_file}
        common.fid_terminus(**param)
        shukey_json= {}
        with open(key_file, 'r') as of:
            shukey_json = json.load(of)

        #2. call data provider to seal data
        sealed_data_url = self.name + ".sealed"
        sealed_output = self.name + ".sealed.output"

        summary = {}

        param = {"data-url":self.data_url,
                "plugin-path":self.plugin_url,
                "sealed-data-url":sealed_data_url,
                "output":sealed_output,
                "use-publickey-file":data_key_file}
        summary['data-url'] = self.data_url
        summary['plugin-path'] = self.plugin_url
        summary['sealed-data-url'] = sealed_data_url
        summary['sealed-output'] = sealed_output

        r = common.fid_data_provider(**param)
        data_hash = self.read_data_hash(sealed_output)
        summary['data-hash'] = data_hash

        print("done seal data with hash: {}, cmd: {}".format(data_hash, r[0]))


        #use first pkey
        key = get_first_key()
        pkey = key['public-key']
        summary['tee-pkey'] = key['public-key']

        #3. call terminusto generate forward message
        forward_result = self.name + ".shukey.foward.json"
        param = {"forward":"",
                "use-privatekey-file":data_key_file,
                "tee-pubkey":pkey,
                "output":forward_result}
        common.fid_terminus(**param);
        data_forward_json = {}
        with open(forward_result, 'r') as of:
            data_forward_json = json.load(of)

        #4. call terminus to generate request
        param_output_url = self.name + "_param.json"
        param = {"request":"",
                "use-privatekey-file":key_file,
                "tee-pubkey":pkey,
                "use-param":self.input,
                "param-format":"text",
                "use-enclave-hash":self.read_parser_hash(),
                "dhash":data_hash,
                "output":param_output_url
                }

        r = common.fid_terminus(**param)
        print("done termins with cmd: {}".format(r[0]))
        param_json = {}
        with open(param_output_url) as of:
            param_json = json.load(of)


        summary['analyzer-pkey-sig'] = param_json["forward-sig"]
        summary['analyzer-pkey'] = param_json['analyzer-pkey']
        summary['analyzer-skey'] = param_json["encrypted-skey"]
        summary['analyzer-input'] = param_json["encrypted-input"]
        summary['program-enclave-hash'] = param_json["program-enclave-hash"]


        result_url = self.name + ".result.encrypted"
        parser_input = {"shu_info":{
            "shu_pkey":shukey_json["public-key"],
            "encrypted_shu_skey":param_json["encrypted-skey"],
            "shu_forward_signature":param_json["forward-sig"],
            "enclave_hash":param_json["program-enclave-hash"]
            },
            "input_data":[{"input_data_url":sealed_data_url,
                "input_data_hash":summary["data-hash"],
                "shu_info":{
                    "shu_pkey":data_shukey_json["public-key"],
                    "encrypted_shu_skey":data_forward_json["encrypted_skey"],
                    "shu_forward_signature":data_forward_json["forward_sig"],
                    "enclave_hash":data_forward_json["enclave_hash"]
                },
                "public-key":data_shukey_json["public-key"],
                "tag":"0"
                }],
            "parser_path":self.parser_url,
            "keymgr_path":common.kmgr_enclave,
            "parser_enclave_hash":param_json["program-enclave-hash"],
            "dian_pkey":param_json["provider-pkey"],
            "model":{
                "model_data":"",
                "public-key":""
                },
            "param":{
                "param_data":param_json["encrypted-input"],
                "public-key":shukey_json["public-key"],
                "allowances":""
            }}

        parser_input_file = self.name + "parser_input.json"
        with open(parser_input_file, "w") as of:
            json.dump(parser_input, of)

        parser_output_file = self.name + "parser_output.json"

        param = {"input":parser_input_file,
                "output":parser_output_file
                }
        r = common.fid_analyzer(**param);
        print("done fid_analyzer with cmd: {}".format(r[0]))
        result_json = {}
        with open(parser_output_file) as of:
            result_json = json.load(of)
        summary['encrypted-result'] = result_json["encrypted_result"]
        summary["result-signature"] = result_json["result_signature"]
        summary["cost-signature"] = result_json["cost_signature"]

        with open(self.name + ".summary.json", "w") as of:
            json.dump(summary, of)

        encrypted_result = summary["encrypted-result"]

        decrypted_result = self.name + ".result"

        param = {"decrypt":"",
                "use-param":encrypted_result,
                "use-privatekey-file":key_file,
                "output":decrypted_result}
        r = common.fid_terminus(**param);

        with open(decrypted_result) as f:
            self.result = f.readlines();


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
    name = "iris"
    data = os.path.join(common.bin_dir, "iris.data")
    parser = os.path.join(common.lib_dir, "iris_parser.signed.so")
    plugin = os.path.join("~/output/iris/output/libiris_csv_reader.so")
    input_param = "123"
    cj = classic_job(name, data, parser, plugin, input_param)
    cj.run()
