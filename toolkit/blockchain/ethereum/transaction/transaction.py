from web3 import Web3
import json
import argparse
import getpass
import threading
import time

import contract
import util
from offline.sign_transaction import sign_transaction


class transaction:
    def __init__(self, host, project_id):
        assert isinstance(host, str)
        assert isinstance(project_id, str)
        self.url = 'https://%s.infura.io/v3/%s' % (host, project_id)
        self.w3 = Web3(Web3.HTTPProvider(self.url))

    def call_contract(self, args):
        sc = contract.contract(self.w3, args.compiled_path)
        return sc.call_contract(args)

    def __send_raw_transaction(self, raw_transaction):
        assert isinstance(raw_transaction, str)
        ret = self.w3.eth.sendRawTransaction(raw_transaction)
        return ret

    def call_and_send_transaction(self, keystore_path, passwd, args):
        call_tx = self.call_contract(args)
        signed_transaction = sign_transaction(keystore_path, passwd, json.loads(call_tx))
        ret = self.__send_raw_transaction(signed_transaction.rawTransaction.hex())
        return ret


def __parse_args():
    parser = argparse.ArgumentParser(description='Web3 python transaction wrapper',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--conf', type=str, default='conf/call_template.json', help='arguments configuration file')
    args = parser.parse_args()
    return args


def main():
    args = __parse_args()
    args = util.read_conf(args.conf, './compiled')

    tx = transaction(args.host, args.project_id)
    op_code = {
        'call': 'tx.call_contract(args)',
        'call_and_send': 'tx.call_and_send_transaction(args.keystore_path, passwd, args)'
    }
    if '_and_send' in args.op_type:
        passwd = getpass.getpass()
    ret = eval(op_code[args.op_type])
    print(ret)


if __name__ == '__main__':
    main()
