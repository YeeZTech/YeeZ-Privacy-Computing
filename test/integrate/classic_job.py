#!/usr/bin/python3
import common
import os
import sys
import json

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

        param = {"data-url":self.data_url,
                "plugin-path":self.plugin_url,
                "sealed-data-url":sealed_data_url,
                "output":sealed_output,
                "sealer-path":common.sealer_enclave}

        r = common.fid_data_provider(**param)
        data_hash = self.read_data_hash(sealed_output)

        print("done seal data with hash: {}, cmd: {}".format(data_hash, r[0]))

        #use first pkey
        keys = common.fid_keymgr_list()
        pkey = ''
        for k, v in keys.items():
            pkey = v
            break;

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
