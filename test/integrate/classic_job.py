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
        '''1. call data provider
        2. call yprepare
        3. call fid_analyzer
        4. call terminus to decrypt'''

        sealed_data_url = self.name + ".sealed"
        sealed_output = self.name + ".sealed.output"

        summary = {}

        param = {"data-url":self.data_url,
                "plugin-path":self.plugin_url,
                "sealed-data-url":sealed_data_url,
                "output":sealed_output,
                "sealer-path":common.sealer_enclave}
        summary['data-url'] = self.data_url
        summary['plugin-path'] = self.plugin_url
        summary['sealed-data-url'] = sealed_data_url
        summary['sealed-output'] = sealed_output
        summary['sealer-path'] = common.sealer_enclave

        r = common.fid_data_provider(**param)
        data_hash = self.read_data_hash(sealed_output)
        summary['data-hash'] = data_hash

        print("done seal data with hash: {}, cmd: {}".format(data_hash, r[0]))

        #use first pkey
        key = get_first_key()
        pkey = key['public-key']
        summary['tee-pkey'] = key['public-key']

        param={"sign":data_hash,
                "sign.hex":"",
                "sign.private-key":key["private-key"]}
        r = common.fid_keymgr(**param)
        rs = r[1].split(':')[1].strip()
        summary['data-hash-signature'] = rs


        sample_json = {"data":[{"data-hash":data_hash, "provider-pkey":pkey}]}

        key_file = self.name + ".key.json"
        param = {"gen-key": "",
                "no-password":"",
                "output":key_file}
        common.fid_terminus(**param)

        sample_json_path = self.name +".sample.json"
        with open(sample_json_path, "w") as of:
            json.dump(sample_json, of)

        param_output_url = self.name + "_param.json"
        param = {"dhash":data_hash,
                "tee-pubkey":pkey,
                "use-param":self.input,
                "param-format":"text",
                "use-enclave-hash":self.read_parser_hash(),
                "output":param_output_url,
                "use-privatekey-file":key_file
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
        param = {"sealed-data-url":sealed_data_url,
                "sealer-path":common.sealer_enclave,
                "parser-path":self.parser_url,
                "keymgr":common.kmgr_enclave,
                "source-type":"json",
                "param-path":param_output_url,
                "result-path":result_url,
                "check-data-hash":data_hash
                }
        r = common.fid_analyzer(**param);
        print("done fid_analyzer with cmd: {}".format(r[0]))
        result_json = {}
        with open(result_url) as of:
            result_json = json.load(of)
        summary['encrypted-result'] = result_json["encrypted-result"]
        summary["result-signature"] = result_json["result-signature"]
        summary["cost-signature"] = result_json["cost-signature"]

        with open(self.name + ".summary.json", "w") as of:
            json.dump(summary, of)

        encrypted_result = ''
        with open(result_url) as f:
            data = json.load(f)
            encrypted_result = data["encrypted-result"]

        decrypted_result = self.name + ".result"

        param = {"decrypt-hex":encrypted_result,
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
