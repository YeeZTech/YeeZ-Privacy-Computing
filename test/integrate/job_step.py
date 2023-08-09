import common
import commonjs
import json


class job_step:
    def gen_key(crypto, shukey_file):
        param = {
            "crypto": crypto,
            "gen-key": "",
            "no-password": "",
            "output": shukey_file
        }
        common.fid_terminus(**param)
        with open(shukey_file, 'r') as of:
            return json.load(of)

    def seal_data(crypto, data_url, plugin_url, sealed_data_url, sealed_output, data_key_file):
        param = {
            "crypto": crypto,
            "data-url": data_url,
            "plugin-path": plugin_url,
            "sealed-data-url": sealed_data_url,
            "output": sealed_output,
            "use-publickey-file": data_key_file
        }
        return common.fid_data_provider(**param)

    def get_first_key(crypto):
        keys = common.fid_keymgr_list(crypto)
        if len(keys) == 0:
            common.fid_keymgr_create("test", crypto)
        keys = common.fid_keymgr_list(crypto)
        pkey = ''
        private_key = ''
        for k, v in keys.items():
            pkey = v
            private_key = common.get_keymgr_private_key(k, crypto)
            break
        return {'public-key': pkey, "private-key": private_key}

    def read_data_hash(fp):
        with open(fp) as f:
            lines = f.readlines()
            for l in lines:
                if l.startswith("data_id"):
                    ks = l.split("=")
                    return ks[1].strip()

    def read_sealed_output(fp, field):
        with open(fp) as f:
            lines = f.readlines()
            for l in lines:
                if l.startswith(field):
                    ks = l.split("=")
                    return ks[1].strip()

    def read_parser_hash(parser_url):
        param = {
            "enclave": parser_url,
            "output": "info.json"
        }
        r = common.fid_dump(**param)
        with open("info.json") as f:
            data = json.load(f)
            return data["enclave-hash"]

    def forward_message(crypto, shukey_file, dian_pkey, enclave_hash, forward_result):
        param = {
            "crypto": crypto,
            "forward": "",
            "use-privatekey-file": shukey_file,
            "tee-pubkey": dian_pkey,
            "output": forward_result
        }
        if enclave_hash:
            param.update({"use-enclave-hash": enclave_hash})
        common.fid_terminus(**param)
        with open(forward_result, 'r') as of:
            return json.load(of)

    def generate_request(crypto, input_param, shukey_file, param_output_url, config):
        param = {
            "crypto": crypto,
            "request": "",
            "use-param": input_param,
            "param-format": "text",
            "use-publickey-file": shukey_file,
            "output": param_output_url
        }
        r = str()
        if 'request-use-js' in config and config['request-use-js']:
            r = commonjs.fid_terminus(**param)
        else:
            r = common.fid_terminus(**param)
        print("done termins with cmd: {}".format(r[0]))
        with open(param_output_url) as of:
            return json.load(of)

    def fid_analyzer(shukey_json, rq_forward_json, enclave_hash, input_data, parser_url, dian_pkey, model, crypto, param_json, allowances, parser_input_file, parser_output_file):
        parser_input = {
            "shu_info": {
                "shu_pkey": shukey_json["public-key"],
                "encrypted_shu_skey": rq_forward_json["encrypted_skey"],
                "shu_forward_signature": rq_forward_json["forward_sig"],
                "enclave_hash": enclave_hash
            },
            "input_data": input_data,
            "parser_path": parser_url,
            "keymgr_path": common.kmgr_enclave[crypto],
            "parser_enclave_hash": enclave_hash,
            "dian_pkey": dian_pkey,
            "model": model,
            "param": {
                "crypto": crypto,
                "param_data": param_json["encrypted-input"],
                "public-key": shukey_json["public-key"],
            }
        }
        if allowances:
            parser_input['param']['allowances'] = allowances
        with open(parser_input_file, "w") as of:
            json.dump(parser_input, of)
        param = {
            "input": parser_input_file,
            "output": parser_output_file
        }
        r = common.fid_analyzer(**param)
        print("done fid_analyzer with cmd: {}".format(r[0]))
        try:
            with open(parser_output_file) as of:
                return json.load(of)
        except Exception as e:
            # result is not json format
            with open(parser_output_file) as of:
                return of.readlines()

    def fid_analyzer_tg(shukey_json, rq_forward_json, algo_shu_info, algo_forward_json, enclave_hash, input_data, parser_url, dian_pkey, model, crypto, param_json, flat_kgt_pkey_list, allowances, parser_input_file, parser_output_file):
        parser_input = {
            "shu_info": {
                "shu_pkey": shukey_json["public-key"],
                "encrypted_shu_skey": rq_forward_json["encrypted_skey"],
                "shu_forward_signature": rq_forward_json["forward_sig"],
                "enclave_hash": enclave_hash
            },
            "algo_shu_info": {
                "shu_pkey": algo_shu_info["public-key"],
                "encrypted_shu_skey": algo_forward_json["encrypted_skey"],
                "shu_forward_signature": algo_forward_json["forward_sig"],
                "enclave_hash": enclave_hash
            },
            "input_intermediate_data": input_data,
            "parser_path": parser_url,
            "keymgr_path": common.kmgr_enclave[crypto],
            "parser_enclave_hash": enclave_hash,
            "dian_pkey": dian_pkey,
            "model": model,
            "param": {
                "crypto": crypto,
                "param_data": param_json["encrypted-input"],
                "public-key": shukey_json["public-key"],
                "algo-public-key": algo_shu_info["public-key"],
                "data-kgt-pkey-list": flat_kgt_pkey_list,
            }
        }
        if allowances:
            parser_input['param']['allowances'] = allowances
        with open(parser_input_file, "w") as of:
            json.dump(parser_input, of)
        param = {
            "input": parser_input_file,
            "output": parser_output_file
        }
        r = common.fid_analyzer(**param)
        print("done fid_analyzer with cmd: {}".format(r[0]))
        try:
            with open(parser_output_file) as of:
                return json.load(of)
        except Exception as e:
            # result is not json format
            with open(parser_output_file) as of:
                return of.readlines()

    def decrypt_result(crypto, encrypted_result, shukey_file, decrypted_result):
        param = {
            "crypto": crypto,
            "decrypt": "",
            "use-param": encrypted_result,
            "use-privatekey-file": shukey_file,
            "output": decrypted_result
        }
        r = common.fid_terminus(**param)
        with open(decrypted_result) as f:
            return f.readlines()

    def decrypt_result_key(crypto, encrypted_result, shukey_file, decrypted_result):
        param = {
            "crypto": crypto,
            "decrypt": "",
            "use-param": encrypted_result,
            "use-privatekey-file": shukey_file,
            "output": decrypted_result
        }
        r = common.fid_terminus(**param)
        with open(decrypted_result, 'rb') as f:
            key = bytearray(f.read())
            return ''.join(format(x, '02x') for x in key)
            # return f.readlines()

    def decrypt_result_with_hex(crypto, encrypted_result, shukey, decrypted_result):
        param = {
            "crypto": crypto,
            "decrypt": "",
            "use-param": encrypted_result,
            "use-privatekey-hex": shukey,
            "output": decrypted_result
        }
        r = common.fid_terminus(**param)
        with open(decrypted_result) as f:
            return f.readlines()

    def encrypt_message(crypto, shukey_file, msg, output):
        param = {
            "crypto": crypto,
            "encrypt": "",
            "use-publickey-file": shukey_file,
            "output": output,
            "use-param": msg
        }
        r = common.fid_terminus(**param)
        with open(output, 'r') as of:
            return of.readlines()[0]

    def sign(crypto, data, param_format, private_key_file, output_url):
        param = {
            "crypto": crypto,
            "sign": "",
            "use-param": data,
            "param-format": param_format,
            "output": output_url,
            "use-privatekey-file": private_key_file
        }
        r = common.fid_terminus(**param)
        with open(output_url, 'r') as of:
            return json.load(of)

    def hash_256(crypto, data, param_format, param_hash_output_url, config):
        param = {
            "crypto": crypto,
            "sha3": "",
            "use-param": data,
            "param-format": param_format,
            "output": param_hash_output_url
        }
        r = str()
        if 'request-use-js' in config and config['request-use-js']:
            r = commonjs.fid_terminus(**param)
        else:
            r = common.fid_terminus(**param)
        with open(param_hash_output_url) as f:
            return f.readlines()[0]

    def generate_allowance(crypto, param_hash, shukey_file, enclave_hash, dian_pkey, data_hash, allowance_output):
        param = {
            "crypto": crypto,
            "allowance": "",
            "use-param": param_hash,
            "use-privatekey-file": shukey_file,
            "use-enclave-hash": enclave_hash,
            "tee-pubkey": dian_pkey,
            "dhash": data_hash,
            "output": allowance_output
        }
        r = common.fid_terminus(**param)
        print("done generate allowance with cmd: {}".format(r[0]))
        with open(allowance_output, 'r') as of:
            return json.load(of)

    def remove_files(file_list):
        [common.execute_cmd('rm -rf {}'.format(f)) for f in file_list]
